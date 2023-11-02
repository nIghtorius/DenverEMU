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
#include "../package/2a03.h"
#include "../audio/expansion/vrc6.h"
#include <SDL_opengl.h>

struct nes_frame_tex {
	int w, h;
	GLvoid *texture;
	GLenum format;
};

class nes_emulator {
private:
	//cpu2a03_fast		* cpu_2a03;
	mainram				* nesram;
	//nes_2a03_joyports	* controllers;
	bool				quit = false;
	nesvideo			* video_out;
	nes_frame_tex		frame;

public:
	//apu				* apu_device;
	package_2a03		* nes_2a03;

	ppu				* ppu_device;
	bus				* mainbus;
	fastclock		  clock;	// trying clock non heap, maybe some speed?
	joypad				* joydefs;
	audio_player		* audio;
	cartridge			* cart;

	nes_emulator();
	~nes_emulator();

	void cold_reset();
	void reset();

	void load_cartridge(const char * filename);
	void load_logo();

	nes_frame_tex	*	returnFrameAsTexture();

	void run_till_frame_ready(void(*callback)(SDL_Event*));
	void fast_run_callback();
	void sync_audio();

	bool hasquit();
	void stop();

	void prepare_frame();
};

