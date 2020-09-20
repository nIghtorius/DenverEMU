/*
	Project Denver

	(c) 2018 Peter Santing.
	This is my second NES emulator. I try to get this as accurate as possible. 
	
*/

// DenverEMU.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "bus.h"
#include "ppu.h"
#include "clock.h"
#include "cpu2a03_fast.h"
#include "mainram.h"
#include "rom.h"
#include "debug6k.h"
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
	*/

	std::cout << "Project Denver version " << DENVER_VERSION << std::endl << "(c) 2018 P. Santing aka nIghtorius" << std::endl << std::endl;
			
	std::cout << "Emulator initializing.." << std::endl;
	
	// setup clock.
	std::cout << "Setting up clock emulation.." << std::endl;
	clock *_DENVER_CLK;
	_DENVER_CLK = new clock();

	// setup new BUS
	std::cout << "Setting up BUS.." << std::endl;
	bus *_DENVER_BUS;
	_DENVER_BUS = new bus();

	// setup CPU
	cpu2a03_fast *_DENVER_CPU;
	_DENVER_CPU = new cpu2a03_fast();
	_DENVER_CPU->mbus = _DENVER_BUS;

	// setup new PPU.
	std::cout << "Setting up PPU.." << std::endl;
	ppu *_DENVER_PPU;
	_DENVER_PPU = new ppu();

	// setup new RAM
	std::cout << "Setting up NES system RAM" << std::endl;
	mainram *_DENVER_INTERNAL_RAM;
	_DENVER_INTERNAL_RAM = new mainram();
	

	// add device(s) to the bus.
	std::cout << "Adding PPU and RAM device to system bus.." << std::endl;
	_DENVER_BUS->registerdevice(_DENVER_PPU);
	_DENVER_BUS->registerdevice(_DENVER_INTERNAL_RAM);
	_DENVER_BUS->registerdevice(_DENVER_CPU);
	
	// add device(s) to the clock.
	std::cout << "Adding PPU and CPU device to clock oscillator.." << std::endl;
	_DENVER_CLK->registerdevice(_DENVER_CPU);
	_DENVER_CLK->registerdevice(_DENVER_PPU);

	// SDL
	std::cout << "Setting up SDL.." << std::endl;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_WM_SetCaption("Denver Project", NULL);
	SDL_Surface * screen = SDL_SetVideoMode(512, 480, 0, 0);
	SDL_Surface * buffer = SDL_CreateRGBSurface(0, 256, 240, 16, 0, 0, 0, 0);

	if (buffer == nullptr) {
		std::cout << "Oh dear.. Null buffer, expect nothing from me. :(.." << std::endl;
	}

	// coldboot the cpu.
	std::cout << "Booting CPU.. " << std::endl;
	//_DENVER_CPU->coldboot();
	
	// run emulation.
	std::cout << "Executing emulation.." << std::endl << std::hex;

	// load test rom.
	std::ifstream romload;
	size_t romsize = 32768;
	romload.open("mario.nes", std::ios::binary | std::ios::in);
	romload.seekg(16);
	char *myrom = (char*)malloc(romsize);
	romload.read(myrom, romsize);
	char *myvrom = (char*)malloc(8192);
	romload.read(myvrom, 8192);
	rom *_DENVER_ROM = new rom();
	vrom *_DENVER_VROM = new vrom();
	_DENVER_ROM->set_rom_data((byte *)myrom, romsize);
	_DENVER_VROM->set_rom_data((byte *)myvrom, 8192);
	_DENVER_BUS->registerdevice(_DENVER_ROM);
	_DENVER_PPU->set_char_rom(_DENVER_VROM);	
	_DENVER_CPU->coldboot();
	_DENVER_CPU->log_register();

	debug6k *_DEBUGGER = new debug6k();
	_DENVER_BUS->registerdevice(_DEBUGGER);
	_DENVER_PPU->configure_vertical_mirror();

	// dump bus data.
	_DENVER_BUS->reportdevices();
	_DENVER_PPU->vbus->reportdevices();
	//_DENVER_CLK->run();

	memset(buffer->pixels, 0xFF, 256 * 240 * 2);

	// before running push in a OAMDMA from 0x01
	//_DENVER_BUS->writememory(0x4014, 0x01);
	bool keeprunning = true;
	while (keeprunning) {
		_DENVER_CLK->step();	
		//_DENVER_PPU->rundevice(_DENVER_CPU->rundevice(1));

		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) keeprunning = false;
		}
		if (_DENVER_PPU->isFrameReady()) {
			SDL_Rect rect;
			rect.x = 0; rect.y = 0; rect.w = 256; rect.h = 240;
			memcpy(buffer->pixels, _DENVER_PPU->getFrameBuffer(), 256 * 240 * 2);
			SDL_BlitSurface(buffer, &rect, screen, 0);			
			SDL_UpdateRect(screen, 0, 0, 0, 0);
		}
	}

	std::cout << "Cpu ticks: " << _DENVER_CPU->ticksdone << std::endl;
	std::cout << "Ppu ticks: " << _DENVER_PPU->ticksdone << std::endl;

	std::cout << "Cpu syncstate: " << _DENVER_CPU->tickstodo << std::endl;
	std::cout << "Ppu syncstate: " << _DENVER_PPU->tickstodo << std::endl;

	// some stuff needed to test -> PPU nametable swapping.
	// swap the pins.
	//_DENVER_PPU->configure_horizontal_mirror();

	for (int i = 0x20; i < 0x30; i++) std::cout << i * 256 << " ==>> " << _DENVER_PPU->vram->compute_addr_from_layout(i * 256) << std::endl;
	
	// dump vram.
	std::ofstream vramdump;
	vramdump.open("vram.dmp", std::ios::binary | std::ios::out);
	vramdump.write((char *)_DENVER_PPU->vram->ram, 0x800);
	vramdump.close();

	return 0;
}
