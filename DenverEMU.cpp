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

	int frames = 0;
	int time = SDL_GetTicks();

	std::cout << "Starting emulation..." << std::endl;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool btrue = true;
	bool *p_open = &btrue;

	glGenTextures(1, &tex);

	nes_emulator * denver = new nes_emulator();

	while (!denver->hasquit()) {
		denver->run_till_frame_ready([](SDL_Event *cbev){
			ImGui_ImplSDL2_ProcessEvent(cbev);
		});

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		//_NESVIDEO->process_ppu_image((std::uint16_t *)_DENVER_PPU->getFrameBuffer());
		nes_frame_tex * nesframe = denver->returnFrameAsTexture();

		// SDL stuff.
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)nesframe->texture);

		{
			ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
			float height = ImGui::GetFrameHeight();

			if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, height, window_flags)) {
				if (ImGui::BeginMenuBar()) {
					if (ImGui::BeginMenu("File")) {

						ImGui::MenuItem("Open file", "Ctrl+O", false);
						if (ImGui::BeginMenu("Recent")) {
							if (ImGui::MenuItem("dtales.nes")) {
								denver->load_cartridge("dtales.nes");
							}
							if (ImGui::MenuItem("mm2.nes")) {
								denver->load_cartridge("mm2.nes");
							}
							if (ImGui::MenuItem("megaman.nes")) {
								denver->load_cartridge("megaman.nes");
							}
							if (ImGui::MenuItem("cv.nes")) {
								denver->load_cartridge("cv.nes");
							}
							ImGui::EndMenu();
						}
						ImGui::Separator();
						if (ImGui::BeginMenu("Save state")) {
							ImGui::MenuItem("state #1");
							ImGui::MenuItem("state #2");
							ImGui::MenuItem("state #3");
							ImGui::MenuItem("state #4");
							ImGui::MenuItem("state #5");
							ImGui::MenuItem("state #6");
							ImGui::EndMenu();
						}
						if (ImGui::BeginMenu("Load state")) {
							ImGui::MenuItem("state #1");
							ImGui::MenuItem("state #2");
							ImGui::MenuItem("state #3");
							ImGui::MenuItem("state #4");
							ImGui::MenuItem("state #5");
							ImGui::MenuItem("state #6");
							ImGui::EndMenu();
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Close", "Ctrl+X", false)) {
							denver->stop();
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Configuration")) {
						if (ImGui::MenuItem("Controllers", NULL, false)) {

						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Emulation")) {
						if (ImGui::MenuItem("Soft reset CPU", "Ctrl+R")) {
							denver->reset();
						}
						if (ImGui::MenuItem("Hard reset CPU", "Ctrl+H")) {
							denver->cold_reset();
						}
						ImGui::Separator();
						if (ImGui::BeginMenu("Debugging")) {
							ImGui::MenuItem("PPU Viewer");
							ImGui::MenuItem("Memory viewer");
							ImGui::MenuItem("Disassembler");
							ImGui::MenuItem("Stack viewer");
							ImGui::MenuItem("APU Viewer");
							if (ImGui::BeginMenu("Denver virtual devices")) {
								ImGui::SeparatorText("CPU BUS");
								for (auto device : denver->mainbus->devices) {
									if (ImGui::MenuItem(device->get_device_descriptor())) {
										device->reset();
									}
								}
								ImGui::SeparatorText("PPU BUS");
								for (auto device : denver->ppu_device->vbus.devices) {
									if (ImGui::MenuItem(device->get_device_descriptor())) {
										device->reset();
									}
								}
								ImGui::EndMenu();
							}
							ImGui::EndMenu();
						}
						ImGui::Separator();
						ImGui::MenuItem("Rewind 5 seconds", "Alt+BkUp");
						ImGui::EndMenu();
					}

					ImGui::EndMenuBar();
				}
				ImGui::End();
			}

			if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags)) {
				if (ImGui::BeginMenuBar()) {
					ImVec4 color = { 0xAA, 0xAA, 0xAA, 0xFF };
					if (io.Framerate < 59.5) color = { 0xFF, 0, 0, 0xFF };
					if (io.Framerate > 61) color = { 0x00, 0xFF, 0, 0xFF };
					ImGui::TextColored(color, "Emulation running. %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
					ImGui::EndMenuBar();
				}
				ImGui::End();
			}

			if (ImGui::BeginViewportSideBar("##NES", viewport, ImGuiDir_Up, io.DisplaySize.y - height*2, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) {
				ImGui::Image((void *)(intptr_t)tex, ImGui::GetContentRegionAvail());
				ImGui::End();
			}
		}

		frames++;	

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
	}

	std::cout << "Emulation ended..." << std::endl;

	// check clean up!
	delete denver;

	return 0;
}
