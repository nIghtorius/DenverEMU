#include "dengui_main.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "../imguifiledialog/ImGuiFileDialog.h"
#include "../package/2a03.h"
#include "../bus/rom/mappers/mapper_nsf.h"
#include "gui_debuggers.h"
#include <iostream>

#pragma warning(disable : 4996)

void	denvergui::render_main (nes_emulator *denver, GLuint tex, denvergui_state *state) {
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	float height = ImGui::GetFrameHeight();

	// file dialogs.
	if (ImGuiFileDialog::Instance()->Display("nesfile", ImGuiWindowFlags_NoCollapse, ImVec2{ 700.0f, 500.0f })) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			denver->nes_2a03->cpu_2a03.stop_execution_log();
			std::cout << "ImGuiFileDialog: " << ImGuiFileDialog::Instance()->GetFilePathName() << std::endl;
			// load the cart.
			denver->load_cartridge(ImGuiFileDialog::Instance()->GetFilePathName().c_str());
			denver->cold_reset();
			if (state->write_exec_log) {
				denver->nes_2a03->cpu_2a03.write_execution_log();
				state->write_exec_log = false;
			}
			std::string newcaption = "Denver - [";
			newcaption += ImGuiFileDialog::Instance()->GetCurrentFileName();
			newcaption += "]";
			SDL_SetWindowTitle(state->mainwin, newcaption.c_str());
		}
		ImGuiFileDialog::Instance()->Close();
	}

	// cpu dialog
	if (state->show_cpu_debugger) {
		render_cpuviewer(denver, state);
	}

	// apu dialog.
	if (state->show_apu_debugger) {
		render_apuviewer(denver, state);
	}

	// ppu dialog
	if (state->show_ppu_debugger) {
		render_ppuviewer(denver, state);
	}
	
	if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, height, window_flags)) 
	{
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open file", "Ctrl+O")) {
					ImGuiFileDialog::Instance()->OpenDialog("nesfile", "Select NES file", "All compatible files{.nes,.nsf},NES roms{.nes},NES music{.nsf}", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
				}

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
				if (ImGui::BeginMenu("Upscalers")) {
					if (ImGui::MenuItem("No upscaling", NULL, (denver->frame_upscaler == 0))) {
						denver->video_out->ClearPostProcessors();
					}
					if (ImGui::MenuItem("Use HQ2X filter", NULL, (denver->frame_upscaler == 1))) {
						denver->video_out->ClearPostProcessors();
						denver->video_out->RegisterPostProcessor(&denver->_hq2x);
					}
					if (ImGui::MenuItem("Use HQ3X filter", NULL, (denver->frame_upscaler == 2))) {
						denver->video_out->ClearPostProcessors();
						denver->video_out->RegisterPostProcessor(&denver->_hq3x);
					}
					if (ImGui::MenuItem("Enable scanlines", NULL, false)) {
						denver->video_out->ClearPostProcessors();
						denver->video_out->RegisterPostProcessor(&denver->_scanlines);
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Audio options")) {
					if (ImGui::MenuItem("Enable sound interpolation", NULL, denver->audio->interpolated)) {
						denver->audio->interpolated = !denver->audio->interpolated;
					}
					if (ImGui::MenuItem("Enable auto attentuation", NULL, !denver->audio->attentuate_lock)) {
						denver->audio->attentuate_lock = !denver->audio->attentuate_lock;
					}
					if (denver->cart->namexp) {
						if (ImGui::MenuItem("NAMCO163 Enhanced Mixing", NULL, denver->cart->namexp->enhanced_mixer)) {
							denver->cart->namexp->enhanced_mixer = !denver->cart->namexp->enhanced_mixer;
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("DebugActions")) {
				if (ImGui::MenuItem("Switch PPU to VER")) {
					denver->ppu_device->vram.resetpins_to_default();
					denver->ppu_device->vram.groundpin(11);
				}
				if (ImGui::MenuItem("Switch PPU to HORIZ")) {
					denver->ppu_device->vram.resetpins_to_default();
					denver->ppu_device->vram.swappins(10, 11);
					denver->ppu_device->vram.groundpin(10);	
				}
				if (ImGui::MenuItem("[mainbus]Dump Device Data to console")) {
					denver->mainbus->reportdevices();
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
					if (ImGui::MenuItem("PPU Viewer")) {
						state->show_ppu_debugger = true;
					}
					ImGui::MenuItem("Memory viewer");
					if (ImGui::MenuItem("CPU viewer")) {
						state->show_cpu_debugger = true;
					}
					ImGui::MenuItem("Stack viewer");
					if (ImGui::MenuItem("Write exec log after new cartridge")) {
						state->write_exec_log = true;
					}
					if (ImGui::MenuItem("APU Viewer")) {
						state->show_apu_debugger = true;
					}
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
						ImGui::SeparatorText("Audio BUS");
						for (auto device : denver->audio->audibles) {
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

	//if (ImGui::BeginViewportSideBar("##NES", viewport, ImGuiDir_Left, io.DisplaySize.y - height * 2, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) 
	ImVec2 startRendering = ImGui::GetMainViewport()->Pos;
	ImVec2 sizeRendering = ImGui::GetMainViewport()->WorkSize;
	startRendering.y += height;
	ImGui::SetNextWindowPos(startRendering);
	ImGui::SetNextWindowSize(sizeRendering);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	if (denver->cart->nsf_mode) {
		if (ImGui::Begin("NES Game", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize || ImGuiWindowFlags_NoInputs))
		{
			if (!denver->cart->nsf_mode) {
				// compute where the image has to come. for now its the 4:3 renderer, later on we implement
				// the 8:7 renderer.

				float scale_x = io.DisplaySize.x / 256.0f; //ImGui::GetContentRegionAvail().x / 256.0f;
				float scale_y = ImGui::GetContentRegionAvail().y / 240.0f;

				float width_x = 0.0f;
				float height_y = 0.0f;

				if (scale_x > scale_y) {
					// scale y dominant.
					width_x = 256.0f * scale_y;
					height_y = 240.0f * scale_y;
				}
				else {
					// scale x dominant.
					width_x = 256.0f * scale_x;
					height_y = 240.0f * scale_x;
				}

				// 8:7 transformation?
				scale_x *= 1.3f;
				float start_x = (ImGui::GetContentRegionAvail().x / 2) - (width_x / 2);
				ImGui::SetCursorPosX(start_x);
				ImGui::Image((void*)(intptr_t)tex, ImVec2{ width_x, height_y });
			}
			else {
				// NSF interface.
				ImGui::Text("Denver NSF Player");
				ImGui::Text("");
				ImGui::Text("Song(s) :");
				ImGui::SameLine();
				ImGui::Text(denver->cart->songname);
				ImGui::Text("Artist :");
				ImGui::SameLine();
				ImGui::Text(denver->cart->artist);
				ImGui::Text("Copyright :");
				ImGui::SameLine();
				ImGui::Text(denver->cart->copyright);
				ImGui::Text("");
				ImGui::Separator();

				// get NSF interface.
				nsfrom* nsfinterface = reinterpret_cast<nsfrom*>(denver->cart->program);

				ImGui::Text("Selected song %d/%d", nsfinterface->state.currentsong, nsfinterface->state.numsongs);
				ImGui::Separator();

				// Media buttons.
				if (ImGui::Button("<<", ImVec2{ 128, 32 })) {
					if (nsfinterface->state.currentsong > 1)
						nsfinterface->state.currentsong--;
					// play the track.
					nsfinterface->initialize(nsfinterface->state.currentsong - 1);
				}
				ImGui::SameLine();
				ImGui::Text("      ");
				ImGui::SameLine();
				if (ImGui::Button(">>", ImVec2{ 128, 32 })) {
					if (nsfinterface->state.currentsong < nsfinterface->state.numsongs)
						nsfinterface->state.currentsong++;
					nsfinterface->initialize(nsfinterface->state.currentsong - 1);
				}
				ImGui::Separator();
				ImGui::Text("Live Sample output");
				int s = (int)denver->audio->final_mux.size();
				float* graph = &denver->audio->final_mux[0];
				float lb = denver->audio->average_mix - 0.7f;
				float hb = denver->audio->average_mix + 0.7f;
				ImGui::PlotLines("Sample", graph, s, 0, NULL, lb, hb, ImVec2{ 0, 160.0f });
				ImGui::Separator();
				ImGui::Text("Expansion Audio:");
				ImGui::SameLine();
				if (denver->cart->namexp) {
					ImGui::Text("NAMCO"); ImGui::SameLine();
				}
				if (denver->cart->vrc6exp) {
					ImGui::Text("VRC6"); ImGui::SameLine();
				}
				if (denver->cart->vrc7exp) {
					ImGui::Text("VRC7"); ImGui::SameLine();
				}
				if (denver->cart->sunexp) {
					ImGui::Text("SUNSOFT"); ImGui::SameLine();
				}
				ImGui::NewLine();


			}
			ImGui::End();
		}
	}
	ImGui::PopStyleVar(1);

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

}