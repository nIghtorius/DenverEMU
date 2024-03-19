/*

	Implementation of the true emulator.
	(c) 2023 P. Santing.

*/

#include "nes.h"
#include "denverlogo.h"
#include <iostream>
#include <sstream>

struct imembuf : std::streambuf
{
	imembuf(const char* base, size_t size)
	{
		char* p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};

struct imemstream : virtual imembuf, std::istream
{
	imemstream(const char* mem, size_t size) :
		imembuf(mem, size),
		std::istream(static_cast<std::streambuf*>(this))
	{
	}
};

nes_emulator::nes_emulator() {
	// setup the emulator.
	mainbus = new bus();
	nes_2a03 = new package_2a03();
	nesram = new mainram();
	ppu_device = new ppu();
	audio = new audio_player();
	joydefs = new joypad();
	nes_2a03->set_joydefs(joydefs);
	cart = nullptr;

	// configure links.
	clock.setdevices(nes_2a03, ppu_device, nullptr);
	mainbus->registerdevice(nes_2a03);
	mainbus->registerdevice(nesram);
	mainbus->registerdevice(ppu_device);
	mainbus->emulate_bus_conflicts(true);

	// configure audio.
	audio->boostspeed = false;
	audio->register_audible_device(&nes_2a03->apu_2a03);

	// configure video.
	video_out = new nesvideo();

	// configure postprocessors.
	
	//video_out->RegisterPostProcessor(&_hq2x);
	//video_out->RegisterPostProcessor(&_scanlines);
	_scanlines.scanlinemode = scanlinetypes::h75;

	// start audio.
	audio->startplayback();
}

std::vector<postprocessor*> nes_emulator::listOfPostProcessors() {
}

nes_emulator::~nes_emulator() {
	// destroy the emulator.
	audio->stopplayback();
	if (cart) delete cart;
	delete audio;
	delete mainbus;	// also kills the linked devices.
	delete joydefs;
	delete video_out;
}

void	nes_emulator::reset() {
	nes_2a03->cpu_2a03.reset();
}

void	nes_emulator::cold_reset() {
	nes_2a03->cpu_2a03.coldboot();
}

void	nes_emulator::run_till_frame_ready(void (*callback)(SDL_Event*)) {
	while (!ppu_device->isFrameReady()) {		
		clock.step();
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (callback) callback(&event);
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if (event.key.keysym.scancode == 0x35) {
					audio->boostspeed = (event.type == SDL_KEYDOWN);
				}
				joydefs->process_kb_event(&event);
				break;
			}
		}
	}
	// frame is ready.. generate frame.
	if (ppu_device) video_out->process_ppu_image((std::uint16_t *)ppu_device->getFrameBuffer());
}

void	nes_emulator::fast_run_callback() {
	clock.run();
}

void	nes_emulator::prepare_frame() {
	if (ppu_device) {
		video_out->process_ppu_image((std::uint16_t *)ppu_device->getFrameBuffer());
		video_out->add_overscan_borders();
	}
}

void	nes_emulator::sync_audio() {
	audio->play_audio();	// blocking call, releases when a frame of audio has been played (16.66ms NTSC)
}

nes_frame_tex * nes_emulator::returnFrameAsTexture() {
	frame.format = GL_RGBA;

	Image* myImage = video_out->getPostImage();

	frame.w = myImage->size.width;
	frame.h = myImage->size.height;
	frame.texture = myImage->image;
	return &frame;
}

bool	nes_emulator::hasquit() {
	return quit;
}

void	nes_emulator::stop() {
	quit = true;
	clock.running = false;
}

void	nes_emulator::load_cartridge(const char * filename) {
	if (cart) delete cart;
	cart = new cartridge(filename, ppu_device, mainbus, audio);
	clock.setdevices(nes_2a03, ppu_device, cart->program);
	nes_2a03->cpu_2a03.coldboot();
}

void	nes_emulator::load_logo() {
	if (cart) delete cart;
	imemstream data((char *)denverlogo, sizeof(denverlogo));
	cart = new cartridge(data, ppu_device, mainbus, audio);
	clock.setdevices(nes_2a03, ppu_device, cart->program);
	nes_2a03->cpu_2a03.coldboot();
}