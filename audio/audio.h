/*

	Denver Audio Class, derived from bus_device
	(c) 2023 P. Santing
	
	What does it do:
		All audio devices are derived from it.

	What it exposes/adds:
	*	std::vector<float> member where samples can be stored.
	
	Also has a class that groups all known audio devices together for a process called
	"final muxing" combining all audio from all linked audio devices together.

*/

#pragma once

#include <vector>
#include "../bus/bus.h"
#include <fstream>
#include <SDL.h>

#define MAX_BUFFER_AUDIO		2048
#define SAMPLE_RATE				44100

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
	std::vector<float> final_mux;
	SDL_AudioDeviceID aud;
	std::int16_t buffer[MAX_BUFFER_AUDIO * 4];	// 4 times the "requirement"
	void	send_sampledata_to_audio_device();
	static void sdl_aud_callback(void * const data, std::uint8_t * const stream, const int len);
public:
	int		sample_rate = SAMPLE_RATE;
	int		samples_in_buffer = 0;
	bool	samples_is_played = false;

	audio_player();
	~audio_player();
	void	register_audible_device(audio_device *dev);
	void	play_audio();
};
