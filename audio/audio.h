/*

	Denver Audio Class, derived from bus_device
	(c) 2023 P. Santing
	
	What does it do:
		All audio devices are derived from it.

	What it exposes/adds:
	*	std::vector<float> member where samples can be stored.
	
	Also has a class that groups all known audio devices together for a process called
	"final muxing" which combines all audio from all linked audio devices together
	and then converts it into a INT16 audio stream to be played back. And then it waits
	a single frame audio playback for frame pacing synchronisation.

	before playing sound. It buffers up to NES_FRAMES frames of audio before playing back.
	keep in mind. When setting a higher SAMPLE_RATE be sure to increase the MAX_BUFFER_AUDIO.
	because the playback routine does not check if the buffers are big enough. at high samples rate
	you do run fast out of buffer. 

	The recommended buffer size is 773 * (SAMPLE_RATE / 44100)

*/

#pragma once

#include <vector>
#include "../bus/bus.h"
#include <SDL.h>

#define SAMPLE_RATE				44100
#define NES_FRAMES				5
#define MAX_BUFFER_AUDIO		773 * (SAMPLE_RATE / 44100)

// classes
class audio_device : public bus_device {
public:
	std::vector<float>			sample_buffer;		// sample buffer.
	int							max_sample_buffer = 1;	// amount of "nes" frames of sound.
	bool						audio_frame_ready = false;
};

class audio_player {
private:
	std::vector<audio_device *>	audibles;
	SDL_AudioDeviceID aud;
	std::int16_t buffer[MAX_BUFFER_AUDIO * 4 * NES_FRAMES];	// 4 times the "requirement"
	void	send_sampledata_to_audio_device();
	static void sdl_aud_callback(void * const data, std::uint8_t * const stream, const int len);
public:
	std::vector<float> final_mux;
	float	average_mix;
	int		sample_rate = SAMPLE_RATE;
	int		samples_in_buffer = 0;
	bool	samples_is_played = false;
	int		samples_per_frame = 0;
	bool	boostspeed = false;

	audio_player();
	~audio_player();
	void	register_audible_device(audio_device *dev);
	void	play_audio();
	bool	has_enough_samples();
	void	startplayback();
	void	stopplayback();
};
