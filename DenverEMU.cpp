/*
	Project Denver

	(c) 2018-2024 Peter Santing.
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

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "denvergui/dengui_main.h"
#include "cpu/tools/2a03_disasm.h"
#include "video/debug_renderer.h"

#include <SDL.h>
#include <GL/glew.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#define		DENVER_VERSION		"0.4 alpha"
#undef main

// defaults
static bool		vsync_enable = false;
static bool		no_audio = false;
static bool		no_audio_emu = false;
static bool		no_expanded_audio = false;
static bool		no_gui = false;
static char		rom_load_startup[512];
static char		shader_load_startup[512];
static bool		fullscreen = false;
static int		upscaler = 0;
static int		width = 512;
static int		height = 480;
static bool		linear_filter = true;
static float	override_scale = -1.0f;

void process_args(int argc, char *argv[]) {
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
			exit(0);
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
	std::cout << "Project Denver version " << DENVER_VERSION << std::endl << "(c) 2018-2024 P. Santing aka nIghtorius" << std::endl;
	std::cout << "Application compiled on " << __DATE__ << " at " << __TIME__ << std::endl << std::endl;

	// start the emulator initialization.
	std::cout << "Emulator initializing.." << std::endl;

	// SDL
	std::cout << "Setting up SDL.." << std::endl;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char* glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	int flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
	if (fullscreen) {
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
	}

	SDL_Window* win = SDL_CreateWindow("Denver NES emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
	
	SDL_GLContext gl_context = SDL_GL_CreateContext(win);
	SDL_GL_MakeCurrent(win, gl_context);
	int vsync = vsync_enable ? 1 : 0;
	if (SDL_GL_SetSwapInterval(vsync)) {
		std::cout << "Unable to set Vsync" << std::endl;
	}

	GLuint tex;
	GLuint ppu_ptable;
	GLuint ppu_ntable;

	// Texture.
	glGenTextures(1, &tex);
	glGenTextures(1, &ppu_ptable);
	glGenTextures(1, &ppu_ntable);

	glBindTexture(GL_TEXTURE_2D, tex);
	
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	
	float ddpi = 0.0f;
	SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
	float display_scale = ddpi / (float)96.0f;
	display_scale = display_scale<(float)1.0f?(float)1.0:display_scale;

	if (override_scale > 0.0f) display_scale = override_scale;

	std::cout << "Window Scale will be " << display_scale << "x\n";

	// Setup Dear ImGui style
	ImFontConfig *fConfig = new ImFontConfig();
	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(display_scale);
	io.Fonts->AddFontFromFileTTF(
		"fonts/denver-ui.ttf", 16 * display_scale, fConfig, io.Fonts->GetGlyphRangesDefault()
	)->Scale = 1.0;
	
	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(win, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);
	glewInit();

	int time = SDL_GetTicks();

	std::cout << "Starting emulation..." << std::endl;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool btrue = true;
	bool *p_open = &btrue;

	nes_emulator * denver = new nes_emulator();
	denver->frame_upscaler = upscaler;

	if (strnlen(shader_load_startup, 512) != 0) {
		denver->use_shader(shader_load_startup);
	}	

	if (no_audio) denver->audio->no_audio = true;
	if (no_audio_emu) denver->nes_2a03->no_apu = true;
	if (no_expanded_audio) denver->audio->no_expanded_audio = true;

	if (strnlen(rom_load_startup, 512) == 0) {
		denver->load_logo();
	}
	else {
		denver->load_cartridge(rom_load_startup);
	}

	denvergui::denvergui_state windowstates;
	windowstates.show_apu_debugger = false;
	windowstates.mainwin = win;

	// setup debug rendering for ppu.
	ppu_debug_vram *ppu_visual_debug = new ppu_debug_vram();
	ppu_visual_debug->set_target_ppu(denver->ppu_device);

	// set the debugger callback for the ppu.
	denver->ppu_device->dbg_callback = ([&]() {
		if (windowstates.show_ppu_debugger) {
			ppu_visual_debug->render_nametable();
		}
	});

	// new call back code. set callback to the ppu.
	denver->ppu_device->callback = ([&] () {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type) {
			case SDL_QUIT:
				denver->clock.running = false;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if (event.key.keysym.scancode == 0x35) {
					denver->audio->boostspeed = (event.type == SDL_KEYDOWN);
				}
				denver->joydefs->process_kb_event(&event);
				break;
			}
		}
		if (!no_gui) {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();
		}
		denver->prepare_frame();
		nes_frame_tex * nesframe = denver->returnFrameAsTexture();

		// ToDo: refactor this into the nes_emulator class (emulator/nes.h)
		glBindTexture(GL_TEXTURE_2D, tex);		
		if (!linear_filter) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nesframe->w, nesframe->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)nesframe->texture);

		if (windowstates.show_ppu_debugger) {
			// render ppu debugger pattern table.
			ppu_visual_debug->render_tilemap(windowstates.pattern_palette);
			glBindTexture(GL_TEXTURE_2D, ppu_ptable);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)ppu_visual_debug->get_patterntable_render());

			// render ppu debugger name table.
			ppu_visual_debug->show_scroll_registers = windowstates.show_scroll_regs;
			ppu_visual_debug->show_ppu_updates_from_cpu = windowstates.show_ppu_updates;

			glBindTexture(GL_TEXTURE_2D, ppu_ntable);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)ppu_visual_debug->get_nametable_render());

			windowstates.pattern_tex = ppu_ptable;
			windowstates.ntable_tex = ppu_ntable;
		}

		int w, h;
		SDL_GetWindowSize(win, &w, &h);
		denver->renderFrameToGL(w, h, tex);

		if (!no_gui) {
			denvergui::render_main(denver, tex, &windowstates);
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
				SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
			}
		}

		SDL_GL_SwapWindow(win);
		denver->sync_audio();
	});

	denver->fast_run_callback();

	std::cout << "Emulation ended..." << std::endl;

	// check clean up!
	delete denver;

	return 0;
}
