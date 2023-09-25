/*
	Project Denver

	(c) 2018 Peter Santing.
	This is my second NES emulator. I try to get this as accurate as possible. 
	
*/

// DenverEMU.cpp : Defines the entry point for the console application.
//
#include "bus/bus.h"
#include "video/ppu.h"
#include "audio/apu.h"
#include "clock/clock.h"
#include "cpu/cpu2a03_fast.h"
#include "bus/ram/mainram.h"
#include "bus/rom/rom.h"
#include "bus/rom/mappers/mapper_001.h"
#include "bus/cart/cart.h"	// cartridge.
#include "audio/audio.h"
#include "video/nesvideo.h"
#include <iostream>
#include <fstream>
#include <malloc.h>
#include <exception>
#include <SDL.h>

#define		DENVER_VERSION		"0.1 beta"
#undef main

int main()
{
	/*
		Print Shield

		THIS IS TESTCODE. NOT FINAL PRODUCT. HENCE THE UGLINESS!
	*/

	std::cout << "Project Denver version " << DENVER_VERSION << std::endl << "(c) 2018 P. Santing aka nIghtorius" << std::endl << std::endl;
			
	std::cout << "Emulator initializing.." << std::endl;
	
	// setup clock.
	std::cout << "Setting up clock emulation.." << std::endl;
	fastclock *_DENVER_CLK;
	_DENVER_CLK = new fastclock();
	//clock *_DENVER_CLK = new clock();

	// setup new BUS
	std::cout << "Setting up BUS.." << std::endl;
	bus *_DENVER_BUS;
	_DENVER_BUS = new bus();

	// setup CPU
	cpu2a03_fast *_DENVER_CPU;
	_DENVER_CPU = new cpu2a03_fast();
	_DENVER_CPU->definememorybus(_DENVER_BUS);

	// setup new PPU.
	std::cout << "Setting up PPU.." << std::endl;
	ppu *_DENVER_PPU;
	_DENVER_PPU = new ppu();

	// setup new APU
	std::cout << "Setting up APU.." << std::endl;
	apu *_DENVER_APU;
	_DENVER_APU = new apu();
	_DENVER_APU->attach_to_memory_bus(_DENVER_BUS);

	// setup video out.
	nesvideo * _NESVIDEO = new nesvideo();

	// setup new RAM
	std::cout << "Setting up NES system RAM" << std::endl;
	mainram *_DENVER_INTERNAL_RAM;
	_DENVER_INTERNAL_RAM = new mainram();
	

	// add device(s) to the bus.
	std::cout << "Adding PPU and RAM device to system bus.." << std::endl;
	_DENVER_BUS->registerdevice(_DENVER_PPU);
	_DENVER_BUS->registerdevice(_DENVER_INTERNAL_RAM);
	_DENVER_BUS->registerdevice(_DENVER_CPU);
	_DENVER_BUS->registerdevice(_DENVER_APU);
	_DENVER_BUS->emulate_bus_conflicts(true);
	
	// add device(s) to the clock.
	std::cout << "Adding PPU and CPU device to clock oscillator.." << std::endl;
	//_DENVER_CLK->registerdevice(_DENVER_CPU);
	//_DENVER_CLK->registerdevice(_DENVER_PPU);
	//_DENVER_CLK->registerdevice(_DENVER_APU);
	_DENVER_CLK->setdevices(_DENVER_CPU, _DENVER_PPU, _DENVER_APU);

	// SDL
	std::cout << "Setting up SDL.." << std::endl;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_Window* win = SDL_CreateWindow("Denver project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 960, SDL_WINDOW_RESIZABLE);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture * tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	// Audio (SDL/denver)
	audio_player *audio = new audio_player();
	audio->register_audible_device(_DENVER_APU);

	// coldboot the cpu.
	std::cout << "Booting CPU.. " << std::endl;
	//_DENVER_CPU->coldboot();
	
	// run emulation.
	std::cout << "Executing emulation.." << std::endl << std::hex;

	// will malloc die here too?
	char * ptr = (char *)malloc (32768);	// for funsies.

	// test cart.h
	cartridge *cart = new cartridge("mm2.nes", _DENVER_PPU, _DENVER_BUS);

	std::cout << "CART mm2 loaded, devices are" << std::endl;
	_DENVER_BUS->reportdevices();
	std::cout << "on APU_BUS it is" << std::endl;
	_DENVER_PPU->vbus.reportdevices();


	_DENVER_CPU->coldboot();
	_DENVER_CPU->log_register();

	// blargg debugger text->console
	/*
	debug6k *_DEBUGGER = new debug6k();
	_DENVER_BUS->registerdevice(_DEBUGGER);
	*/
	
	
	// before running push in a OAMDMA from 0x01
	//_DENVER_BUS->writememory(0x4014, 0x01);
	bool keeprunning = true;

	int frames = 0;
	int time = SDL_GetTicks();

	std::cout << "Starting emulation..." << std::endl;

	while (keeprunning) {
		_DENVER_CLK->step();	
		//_DENVER_PPU->rundevice(_DENVER_CPU->rundevice(1));

		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) keeprunning = false;
		}
		if (_DENVER_PPU->isFrameReady()) {
			_NESVIDEO->process_ppu_image((std::uint16_t *)_DENVER_PPU->getFrameBuffer());
			
			// SDL stuff.
			SDL_UpdateTexture(tex, NULL, (void *)_NESVIDEO->getFrame(), 512);
			SDL_RenderCopy(rend, tex, NULL, NULL);
			//SDL_RenderCopyEx(rend, tex, NULL, NULL, 15, NULL, SDL_RendererFlip{ SDL_FLIP_VERTICAL });
			SDL_RenderPresent(rend);
			audio->play_audio();

			frames++;

			//SDL_Delay(16); 
		}
		/*
		if (frames == 60) {
			time = SDL_GetTicks() - time;
			float fps = 60 / ((float)time / 1000);
			std::cout << "Current FPS: " << std::dec << fps << std::endl;
			frames = 0;
			time = SDL_GetTicks();
		}*/
	}

	std::cout << "Emulation ended..." << std::endl;

	// check clean up!
	// free the ppu unit.
	delete _DENVER_PPU;

	return 0;
}
