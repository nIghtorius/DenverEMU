/*
	
	nes.h

	The class that brings it all together.
	processing sdl events, etc.


*/

#pragma once

#include "../package/2a03.h"
#include "../bus/ram/mainram.h"
#include "../bus/cart/cart.h"
#include "../audio/audio.h"
#include "../clock/clock.h"
#include "../video/ppu.h"
#include "../video/nesvideo.h"
#include "../video/postprocessing/hqxpost.h"
#include "../video/postprocessing/scanlines.h"

#include <GL/glew.h>
#include <SDL_opengl.h>
#include <iostream>

#define	DENVER_NO_UPSCALER			0
#define DENVER_HQ2X_UPSCALER		1
#define	DENVER_HQ3X_UPSCALER		2

struct nes_frame_tex {
	int w, h;
	GLvoid *texture;
	GLenum format;
};

class nes_emulator {
private:
	mainram				* nesram;
	nes_frame_tex		frame;
	bool				quit = false;
	uint64_t			time;

public:
	nesdb* db = nullptr;
	nesvideo* video_out;
	package_2a03		* nes_2a03;
	ppu					* ppu_device;
	bus					* mainbus;
	fastclock			clock;	// trying clock non heap, maybe some speed?
	joypad				* joydefs;
	audio_player		* audio;
	cartridge			* cart;

	// postprocessors.
	hq2x				_hq2x;
	hq3x				_hq3x;
	scanlines			_scanlines;

	int					frame_upscaler = DENVER_NO_UPSCALER;			// depricated; does nothing now.

	// shaders.
	GLuint				shader = 0;

	nes_emulator();
	~nes_emulator();

	void cold_reset();
	void reset();

	void use_shader(const char* filename);

	void load_cartridge(const char * filename);
	void load_logo();

	nes_frame_tex	*	returnFrameAsTexture();

	void run_till_frame_ready(void(*callback)(SDL_Event*));
	void fast_run_callback();
	void sync_audio();

	bool hasquit();
	void stop();

	void prepare_frame();
	void renderFrameToGL(const int windowWidth, const int windowHeight, const GLuint tex);
};
