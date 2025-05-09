/*
	Project Denver

	(c) 2018-2025 Peter Santing.
	This is my second NES emulator. I try to get this as accurate as possible. 
	
*/

#pragma warning(disable : 4996)

// DenverEMU.cpp : Defines the entry point for the console application.
//
#include "emulator/nes.h"
#include <iostream>
#include <fstream>
#include <exception>
#include <string>

#include "denvergui/dengui_main.h"

#include <SDL.h>
#include <GL/glew.h>

#include "emulator/denver_config.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

// Windows console output (UTF-8)
#ifdef _WIN32
#include <Windows.h>
#endif

#define		DENVER_VERSION		"0.9 alpha"
#undef main

// defaults
bool		vsync_enable = false;
bool		no_audio = false;
bool		no_audio_emu = false;
bool		no_expanded_audio = false;
bool		no_gui = false;
char		rom_load_startup[512];
char		shader_load_startup[512];
bool		fullscreen = false;
int			upscaler = 0;
int			width = 512;
int			height = 480;
bool		linear_filter = true;
float		override_scale = -1.0f;
int			p1_controller = 0;
int			p2_controller = 1;
bool		show_controllers = false;
bool		do_exec_trace = false;
bool		generate_readable_stacktrace = false;

std::vector<std::string> shaderList;

static void process_args(int argc, char* argv[]) {
	for (int i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-?") == 0) || (strcmp(argv[i], "/?") == 0)) {
			// show help.
			std::cout << "Command line options:\n";
			std::cout << "--vsync, enables vsync.\n--no-vsync, disables vsync.\n--no-audio, disable audio playback, also uncapped speed.\n";
			std::cout << "--no-apu-emulation, disables APU emulation\n";
			std::cout << "--no-expanded-audio, disables expansion audio emulation\n";
			std::cout << "--no-gui, disabled GUI, requires a rom file to be specified with --rom-file\n";
			std::cout << "--rom-file <romfile>, ROM file to run\n";
			std::cout << "--fullscreen, runs emulator in fullscreen mode\n";
			std::cout << "--upscaler <upscaler>, selects upscaler, functionality is removed, does nothing now\n";
			std::cout << "--width <width>, --height <height>, sets width and height of the window or fullscreen resolution\n";
			std::cout << "--linear-filter, enables linear filtering\n";
			std::cout << "--no-linear-filter, disables linear filtering\n";
			std::cout << "--load-shader <shader>, loads a fragment shader (OpenGL)\n";
			std::cout << "--override-scale <factor>, overrides display scaling (DPI awareness), expects factors. ex: 2 = 200%\n";
			std::cout << "--show-controllers, list all detected controllers and exit\n";
			std::cout << "--select-controller-id-port1, selects a gamecontroller as port #1\n";
			std::cout << "--select-controller-id-port2, selects a gamecontroller as port #2\n";
			std::cout << "--executiontrace\n";
			std::cout << "--make-stacktrace\n";
			exit(0);
		}
		if (strcmp(argv[i], "--executiontrace") == 0) {
			std::cout << "WARNING! Running execution trace, log file grows fast!\n";
			do_exec_trace = true;
		}

		if (strcmp(argv[i], "--make-stacktrace") == 0) {
			std::cout << "Require more memory! debug uses only. Generate readable stacktraces.\n";
			generate_readable_stacktrace = true;
		}

		if (strcmp(argv[i], "--show-controllers") == 0) {
			show_controllers = true;
		}
		if (strcmp(argv[i], "--select-controller-id-port1") == 0) {
			if (i + 1 <= argc) {
				p1_controller = atoi(argv[i + 1]);
				std::cout << "PORT #1 controller ID set to " << p1_controller << "\n";
			}
		}
		if (strcmp(argv[i], "--select-controller-id-port2") == 0) {
			if (i + 1 <= argc) {
				p2_controller = atoi(argv[i + 1]);
				std::cout << "PORT #2 controller ID set to " << p2_controller << "\n";
			}
		}
		if (strcmp(argv[i], "--vsync") == 0) {
			vsync_enable = true;
			std::cout << "--vsync option, enabling v-sync" << std::endl;
		}
		if (strcmp(argv[i], "--no-vsync") == 0) {
			vsync_enable = false;
			std::cout << "--no-vsync, disabling v-sync" << std::endl;
		}
		if (strcmp(argv[i], "--no-audio") == 0) {
			no_audio = true;
			std::cout << "--no-audio, disable audio output. emulation runs uncapped, or enable vsync" << std::endl;
		}
		if (strcmp(argv[i], "--no-apu-emulation") == 0) {
			no_audio_emu = true;
			no_audio = true;
			std::cout << "--no-apu-emulation, audio emulation disabled." << std::endl;
		}
		if (strcmp(argv[i], "--no-expanded-audio") == 0) {
			no_expanded_audio = true;
			std::cout << "--no-expanded-audio, audio expansion emulation disabled." << std::endl;
		}
		if (strcmp(argv[i], "--no-gui") == 0) {
			no_gui = true;
			std::cout << "--no-gui, gui disabled. requires --rom-file <rom>\n";
		}
		if (strcmp(argv[i], "--rom-file") == 0) {
			if (i + 1 <= argc) {
				strncpy(rom_load_startup, argv[i + 1], 512);
			}
		}
		if (strcmp(argv[i], "--fullscreen") == 0) {
			std::cout << "--fullscreen, full screen enabled.\n";
			fullscreen = true;
		}
		if (strcmp(argv[i], "--upscaler") == 0) {
			if (i + 1 <= argc) {
				upscaler = atoi(argv[i + 1]);
				std::cout << "Upscaler set to: " << (int)upscaler << "\n";
			}
		}
		if (strcmp(argv[i], "--width") == 0) {
			if (i + 1 <= argc) {
				width = atoi(argv[i + 1]);
			}
		}
		if (strcmp(argv[i], "--height") == 0) {
			if (i + 1 <= argc) {
				height = atoi(argv[i + 1]);
			}
		}
		if (strcmp(argv[i], "--linear-filter") == 0) {
			linear_filter = true;
		}
		if (strcmp(argv[i], "--no-linear-filter") == 0) {
			linear_filter = false;
		}
		if (strcmp(argv[i], "--load-shader") == 0) {
			if (i + 1 <= argc) {
				strncpy(shader_load_startup, argv[i + 1], 512);
			}
		}
		if (strcmp(argv[i], "--override-scale") == 0) {
			if (i + 1 <= argc)
				override_scale = (float)atof(argv[i+1]);
		}
	}
}

int main(int argc, char *argv[])
{
	rom_load_startup[0] = 0x0;

#ifdef _WIN32
	SetConsoleOutputCP(65001);
#endif

	if (argc > 1) process_args(argc, argv);

	if (no_gui) {
		if (strnlen(rom_load_startup, 512) == 0) {
			std::cout << "--no-gui, requires --rom-file <filename> to specified\n";
			return 128;
		}
	}
	/*
		Print Shield
	*/
	std::cout << "Project Denver version " << DENVER_VERSION << std::endl << "(c) 2018-2025 P. Santing aka nIghtorius" << std::endl;
	std::cout << "Application compiled on " << __DATE__ << " at " << __TIME__ << std::endl << std::endl;

	// start the emulator initialization.
	std::cout << "Emulator initializing.." << std::endl;

	// init SDL
	SDL_SetHint(SDL_HINT_JOYSTICK_THREAD, "1");
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);

	// initialize denver and SDL stuff.
	nes_emulator* denver = new nes_emulator();

	// start the emulator. (which will initialize callbacks, video, audio, controllers, etc)
	denver->start();

	std::cout << "Emulation ended..." << std::endl;

	// check clean up!
	delete denver;

	return 0;
}
