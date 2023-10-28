/*
	Project Denver

	(c) 2018 Peter Santing.
	This is my second NES emulator. I try to get this as accurate as possible. 
	
*/

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
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#define		DENVER_VERSION		"0.2 alpha"
#undef main

int main()
{
	/*
		Print Shield
	*/

	std::cout << "Project Denver version " << DENVER_VERSION << std::endl << "(c) 2018 P. Santing aka nIghtorius" << std::endl << std::endl;
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

	SDL_Window* win = SDL_CreateWindow("Denver - NES Emulator - Alpha version", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 960, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_GLContext gl_context = SDL_GL_CreateContext(win);
	SDL_GL_MakeCurrent(win, gl_context);
	if (SDL_GL_SetSwapInterval(0)) {
		std::cout << "Unable to disable Vsync" << std::endl;
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

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

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

	int time = SDL_GetTicks();

	std::cout << "Starting emulation..." << std::endl;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool btrue = true;
	bool *p_open = &btrue;

	nes_emulator * denver = new nes_emulator();
	denver->load_logo();

	denvergui::denvergui_state windowstates;
	windowstates.show_apu_debugger = false;

	// setup debug rendering for ppu.
	ppu_debug_vram *ppu_visual_debug = new ppu_debug_vram();
	ppu_visual_debug->set_target_ppu(denver->ppu_device);

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
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		denver->prepare_frame();
		nes_frame_tex * nesframe = denver->returnFrameAsTexture();
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)nesframe->texture);

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
			ppu_visual_debug->render_nametable();
			glBindTexture(GL_TEXTURE_2D, ppu_ntable);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)ppu_visual_debug->get_nametable_render());

			windowstates.pattern_tex = ppu_ptable;
			windowstates.ntable_tex = ppu_ntable;
		}

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

		SDL_GL_SwapWindow(win);
		denver->sync_audio();
	});

	denver->fast_run_callback();

	std::cout << "Emulation ended..." << std::endl;

	// check clean up!
	delete denver;

	return 0;
}
