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
#include "controller/joypad.h"
#include <iostream>
#include <fstream>
#include <malloc.h>
#include <exception>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

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

	SDL_Window* win = SDL_CreateWindow("Denver project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 960, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_GLContext gl_context = SDL_GL_CreateContext(win);
	SDL_GL_MakeCurrent(win, gl_context);
	SDL_GL_SetSwapInterval(0); // Enable vsync

	GLuint tex;
	// Texture.
	glGenTextures(1, &tex);
	//glActiveTexture(GL_TEXTURE1);
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


	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	//SDL_Texture * tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	
	// Audio (SDL/denver)
	audio_player *audio = new audio_player();
	audio->register_audible_device(_DENVER_APU);

	// coldboot the cpu.
	std::cout << "Booting CPU.. " << std::endl;
	//_DENVER_CPU->coldboot();

	joypad joys;
	joys.set_default_configs();
	nes_2a03_joyports * _DENVER_CTRL = new nes_2a03_joyports(&joys);
	_DENVER_BUS->registerdevice(_DENVER_CTRL);	// add to the bus.

	// run emulation.
	std::cout << "Executing emulation.." << std::endl << std::hex;

	// test cart.h
	cartridge *cart = new cartridge("mario.nes", _DENVER_PPU, _DENVER_BUS);

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

	audio->startplayback();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool btrue = true;
	bool *p_open = &btrue;

	glGenTextures(1, &tex);

	while (keeprunning) {

		_DENVER_CLK->step();	
		//_DENVER_PPU->rundevice(_DENVER_CPU->rundevice(1));

		SDL_Event event;
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				keeprunning = false;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				joys.process_kb_event(&event);
				break;
			}
		}

		if (_DENVER_PPU->isFrameReady()) {
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			bool first_time = true;

			_NESVIDEO->process_ppu_image((std::uint16_t *)_DENVER_PPU->getFrameBuffer());

			// SDL stuff.
			//SDL_UpdateTexture(tex, NULL, (void *)_NESVIDEO->getFrame(), 512);
			//SDL_RenderCopy(rend, tex, NULL, NULL);
			//SDL_RenderCopyEx(rend, tex, NULL, NULL, 15, NULL, SDL_RendererFlip{ SDL_FLIP_VERTICAL });
			//SDL_RenderPresent(rend);
			//glActiveTexture(GL_TEXTURE1);
			//glDeleteTextures(1, &tex);
			//glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)_NESVIDEO->getFrame());
			// 1. Menu bar
			{
				static bool opt_fullscreen = true;
				static bool opt_padding = false;
				static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

				// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
				// because it would be confusing to have two docking targets within each others.
				ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
				if (opt_fullscreen)
				{
					const ImGuiViewport* viewport = ImGui::GetMainViewport();
					ImGui::SetNextWindowPos(viewport->WorkPos);
					ImGui::SetNextWindowSize(viewport->WorkSize);
					ImGui::SetNextWindowViewport(viewport->ID);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
					window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
					window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
				}
				else
				{
					dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
				}

				// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
				// and handle the pass-thru hole, so we ask Begin() to not render a background.
				if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
					window_flags |= ImGuiWindowFlags_NoBackground;

				// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
				// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
				// all active windows docked into it will lose their parent and become undocked.
				// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
				// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
				if (!opt_padding)
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				ImGui::Begin("DockSpace Demo", p_open, window_flags);
				if (!opt_padding)
					ImGui::PopStyleVar();

				if (opt_fullscreen)
					ImGui::PopStyleVar(2);

				// Submit the DockSpace
				ImGuiIO& io = ImGui::GetIO();
				if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
				{
					ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
					ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
					/*
					if (first_time) {
						first_time = false;
						ImGui::DockBuilderRemoveNode(dockspace_id);
						ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
						ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());
						ImGuiID dockspace_main_id = dockspace_id;
						ImGui::DockBuilderDockWindow("Hello, world!", dockspace_main_id);
					}*/
				}
				else
				{
					//ShowDockingDisabledMessage();
				}

				if (ImGui::BeginMenuBar()) {
					if (ImGui::BeginMenu("File")) {
						if (ImGui::MenuItem("Close", "Ctrl+X", false)) {
							keeprunning = false;
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}

				ImGui::End();
			}


			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("NES");                          // Create a window called "Hello, world!" and append into it.	

				ImGui::Image((void *)(intptr_t)tex, ImGui::GetContentRegionAvail());

				ImGui::End();
			}

			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.				

				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
				//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				//ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
				ImGui::End();
			}

			frames++;	

			ImGui::Render();
			glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
			glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
			//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
				SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
			}

			SDL_GL_SwapWindow(win);
			audio->play_audio();


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
		// Rendering
	}

	std::cout << "Emulation ended..." << std::endl;

	// check clean up!
	// free the ppu unit.
	delete _DENVER_PPU;

	return 0;
}
