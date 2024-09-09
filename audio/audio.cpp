#include "audio.h"
#include <vector>
#include <SDL.h>
#include <iostream>	

audio_player::audio_player() {
	// setup audio (SDL)
	SDL_AudioSpec desired;
	desired.samples = MAX_BUFFER_AUDIO; // int16 is 2 bytes.
	desired.channels = 1;
	desired.format = AUDIO_F32;
	desired.freq = SAMPLE_RATE;
	desired.silence = 0;
	desired.callback = audio_player::sdl_aud_callback;
	desired.userdata = this;

	aud = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
}

void	audio_player::sdl_aud_callback(void * const data, std::uint8_t* const stream, const int len)
{
	// get *audio_player* object from data.
	const auto aud_player_callback = reinterpret_cast<audio_player*>(data);

	// check buffer is full enough, otherwise mute.
	SDL_memset(stream, aud_player_callback->de_pop_sample, len);

	if (aud_player_callback->samples_in_buffer < MAX_BUFFER_AUDIO) return;

	// copy data to stream.
	memcpy(stream, (void *)&aud_player_callback->buffer[0], len);
	aud_player_callback->de_pop_sample = aud_player_callback->buffer[(len/sizeof(float)) - 1];

	// shift buffers
	memcpy(&aud_player_callback->move_buffer[0], (void *)&aud_player_callback->buffer[MAX_BUFFER_AUDIO], (aud_player_callback->samples_in_buffer - MAX_BUFFER_AUDIO) * sizeof(float));
	memcpy((void *)&aud_player_callback->buffer[0], &aud_player_callback->move_buffer[0], (aud_player_callback->samples_in_buffer - MAX_BUFFER_AUDIO) * sizeof(float));

	// drop length.
	aud_player_callback->samples_in_buffer -= MAX_BUFFER_AUDIO;
}

audio_player::~audio_player() {
	SDL_CloseAudioDevice(aud);
}

bool	audio_player::has_enough_samples() {
	// checks NES_FRAMES * samples_per_frame is met.
	return samples_in_buffer >= samples_per_frame * NES_FRAMES;
}

void	audio_player::register_audible_device(audio_device *dev) {
	std::cout << "Registering audio device: " << dev->get_device_descriptor() << "\n";
	audibles.push_back(dev);
}

void	audio_player::unregister_audible_device(audio_device *dev) {
	if (audibles.size() == 0) return;
	for (int i = 0; i < audibles.size(); i++) {
		if (dev == audibles[i]) {
			std::cout << "Unregistering audio device: " << dev->get_device_descriptor() << "\n";
			audibles.erase(audibles.begin() + i);
			return;
		}
	}
}

void	audio_player::play_audio() {
	// test if devices are registered, when no. No need to play as there is none.
	if (audibles.size() == 0) return;
	// test if audio frame is ready. test only first device they should become ready together.
	if (!audibles[0]->audio_frame_ready) return;

	// audio disabled?
	if (no_audio) {
		for (auto audible : audibles) {
			audible->sample_buffer.clear();
			audible->audio_frame_ready = false;
		}
		return;
	}

	// everything is ready. let's mux everything together.
	final_mux.clear();

	float avg_center = 0.0f;
	bool increaseattentuate = true;

	for (int i = 0; i < audibles[0]->sample_buffer.size(); i++) {
		float input = 0.0f;
		for (auto audible : audibles) {
			if (i < audible->sample_buffer.size())
				input += audible->sample_buffer[i];
			if (no_expanded_audio) break;
		}
		avg_center += input;

		// balance volume.
		// peaking > lock to 1.0f
		if (input * attentuate > 1.0f) {
			attentuate = 1.0f / (input * attentuate);
			increaseattentuate = false;
		}
		// below 90% incremently increase volume.
		if (input * attentuate >= 0.9f) {
			increaseattentuate = false;
		}
		// below 5% do not attentuate is silence.
		if (input * attentuate <= 0.05f) {
			increaseattentuate = false;
		}

		final_mux.push_back(input);
	}
	
	if (increaseattentuate) 
		if (attentuate < max_attentuate) attentuate += 0.005f;

	if (attentuate_lock) attentuate = .95f;	// lock dynamic attentuation @ 0.90x

	avg_center /= (float)audibles[0]->sample_buffer.size();
	average_mix += avg_center; average_mix /= 2;	
	
	for (auto audible : audibles) {
		audible->sample_buffer.clear();
		audible->audio_frame_ready = false;
	}

	send_sampledata_to_audio_device();
}

void	audio_player::bWorthFilter(const float input, float& output) {
	output = input * bw.a0 + bw.z1;
	bw.z1 = input * bw.a1 + bw.z2 - bw.b1 * output;
	bw.z2 = input * bw.a2 + bw.z3 - bw.b2 * output;
	bw.z3 = input * bw.a3 - bw.b3 * output;
	bw.p0 = bw.p1; bw.p1 = bw.p2; bw.p2 = input;
}

void	audio_player::simpleLowpass(const float input, float& output) {
	if (output == input) return;
	output += 0.5f * (input - output);
	// clamp output when near target "input"
	if ((output - CLAMP_LP < input) && (output + CLAMP_LP > input)) output = input;
}

void	audio_player::send_sampledata_to_audio_device() {
	float samples_to_target = NES_CLOCK_SPEED_NTSC / (float)sample_rate;
	if (boostspeed) samples_to_target = (NES_CLOCK_SPEED_NTSC*4) / (float)sample_rate;
	// supersampling. final version with lowpass.
	float	samples = 0;
	int		outsamples = 0;
	int sib_bk = samples_in_buffer;

	while (samples < final_mux.size()) {
		float sample = 0;
		if (samples + samples_to_target > final_mux.size()) samples_to_target = final_mux.size() - samples;
		sample = final_mux[(int)trunc(samples)];
		bWorthFilter(sample, lpout);
		buffer[samples_in_buffer + outsamples] = (interpolated ? lpout : sample) * attentuate;
		samples += samples_to_target;
		outsamples++;
	}
	outsamples--;
	if (outsamples >= samples_per_frame) samples_per_frame = outsamples;

	// buffer has the data and outsamples has the number of samples.
	samples_in_buffer += outsamples;

	// flush buffer.
	while (has_enough_samples()) {
		SDL_PumpEvents();
	}
}

void	audio_player::startplayback() {
	SDL_PauseAudioDevice(aud, 0);	
}

void	audio_player::stopplayback() {
	SDL_PauseAudioDevice(aud, 1);
}

void	audio_device::set_debug_data() {

}