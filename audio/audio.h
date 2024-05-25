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
#include <cmath>


#define SAMPLE_RATE				48000
#define NES_FRAMES				5
#define MAX_BUFFER_AUDIO		842 //773 / (44100 / SAMPLE_RATE)
#define	NES_CLOCK_SPEED_NTSC	1789772			// clockspeed in hertz

#define ALPHA_LP				0.4
#define CLAMP_LP				0.005

constexpr float Wc = 0.2f;

// bworth filter.
struct bworth {
	float K = (float)std::tan(M_PI * Wc);
	float norm = 1 / (K * K * K + 2 * K * K + 2 * K + 1);
	float a0 = K * K * K * norm;
	float a1 = 3 * a0;
	float a2 = a1;
	float a3 = a0;
	float b1 = (3 * K * K * K + 2 * K * K - 2 * K - 3) * norm;
	float b2 = (3 * K * K * K - 2 * K * K - 2 * K + 3) * norm;
	float b3 = (K * K * K - 2 * K * K + 2 * K - 1) * norm;
	float z1 = 0;
	float z2 = 0;
	float z3 = 0;
	float p0 = 0;
	float p1 = 0;
	float p2 = 0;
};

// classes
class audio_device : public bus_device {
public:
	std::vector<float>			sample_buffer;		// sample buffer.
	int							max_sample_buffer = 1;	// amount of "nes" frames of sound.
	bool						audio_frame_ready = false;
	virtual void				set_debug_data();
};

class audio_player {
private:
	SDL_AudioDeviceID aud;
	bworth	bw;
	float	lpout = 0.0f;
	float   buffer[MAX_BUFFER_AUDIO * 4 * NES_FRAMES];	// 4 times the "requirement"
	float   move_buffer[MAX_BUFFER_AUDIO * 4 * NES_FRAMES]; // shift/move buffer.
	void	send_sampledata_to_audio_device();
	static void sdl_aud_callback(void * const data, std::uint8_t * const stream, const int len);
	void	bWorthFilter(const float input, float& output);
	void	simpleLowpass(const float input, float& output);
public:
	std::vector<audio_device *>	audibles;
	std::vector<float> final_mux;
	float	de_pop_sample = 0.0f;
	float	average_mix;
	int		sample_rate = SAMPLE_RATE;
	int		samples_in_buffer = 0;
	bool	samples_is_played = false;
	int		samples_per_frame = 0;
	bool	boostspeed = false;
	bool	no_audio = false;
	bool	no_expanded_audio = false;
	bool	interpolated = true;
	float	attentuate = 1.0f;
	bool	attentuate_lock = true;
	float	max_attentuate = 1.05f;

	audio_player();
	~audio_player();
	void	register_audible_device(audio_device *dev);
	void	unregister_audible_device(audio_device *dev);
	void	play_audio();
	bool	has_enough_samples();
	void	startplayback();
	void	stopplayback();
};