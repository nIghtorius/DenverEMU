/*
	
	nes.h

	The class that brings it all together.
	processing sdl events, etc.


*/

#pragma once

#include "../bus/bus.h"
#include "../bus/ram/mainram.h"
#include "../bus/cart/cart.h"
#include "../audio/apu.h"
#include "../audio/audio.h"
#include "../clock/clock.h"
#include "../cpu/cpu2a03_fast.h"
#include "../video/ppu.h"
#include "../video/nesvideo.h"
#include "../controller/joypad.h"
#include <SDL_opengl.h>

struct nes_frame_tex {
	int w, h;
	GLvoid *texture;
	GLenum format;
};

class nes_emulator {
private:
	cartridge			* cart;
	cpu2a03_fast		* cpu_2a03;
	mainram				* nesram;
	audio_player		* audio;
	joypad				* joydefs;
	nes_2a03_joyports	* controllers;
	bool				quit = false;
	nesvideo			* video_out;
	nes_frame_tex		frame;

public:
	apu				* apu_device;
	ppu				* ppu_device;
	bus				* mainbus;
	fastclock		  clock;	// trying clock non heap, maybe some speed?

	nes_emulator();
	~nes_emulator();

	void cold_reset();
	void reset();

	void load_cartridge(const char * filename);

	nes_frame_tex	*	returnFrameAsTexture();

	void run_till_frame_ready(void(*callback)(SDL_Event*));
	void sync_audio();

	bool hasquit();
	void stop();
};

