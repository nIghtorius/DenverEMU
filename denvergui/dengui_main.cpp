﻿#include "dengui_main.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "../imguifiledialog/ImGuiFileDialog.h"
#include "../package/2a03.h"
#include "../bus/rom/mappers/mapper_nsf.h"
#include "../bus/rom/disksystem//fds.h"
#include "gui_debuggers.h"
#include <iostream>

#pragma warning(disable : 4996)
#define USE_PLACES_DEVICES


extern std::vector<std::string> shaderList;


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
			state->lastpath = ImGuiFileDialog::Instance()->GetCurrentPath() + "/";
			// load the cart.
			denver->load_cartridge(ImGuiFileDialog::Instance()->GetFilePathName().c_str());
			denver->cold_reset();
			if (state->write_exec_log) {
				denver->nes_2a03->cpu_2a03.write_execution_log();
				state->write_exec_log = false;
			}
			std::string newcaption = "Denver NES emulator - [";
			newcaption += ImGuiFileDialog::Instance()->GetCurrentFileName();
			newcaption += "]";
			SDL_SetWindowTitle(state->mainwin, newcaption.c_str());
		}
		ImGuiFileDialog::Instance()->Close();
	}

	// when gui starts rendering. Check if a new NES rom needs to be loaded..
	if (state->romChange) {
		state->romChange = false;
		std::string newcaption = "Denver NES emulator - [";
		newcaption += state->changeRomTo;
		newcaption += "]";
		denver->load_cartridge(state->changeRomTo.c_str());
		denver->cold_reset();
		SDL_SetWindowTitle(state->mainwin, newcaption.c_str());
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

	// other debuggers.
	for (auto device : denver->mainbus->devices) {
		if (device->debugger.show_debugger) {
			render_debugviewer(&device->debugger, &device->debugger.show_debugger, device->get_device_descriptor());
		}
	}
	for (auto device : denver->ppu_device->vbus.devices) {
		if (device->debugger.show_debugger) {
			render_debugviewer(&device->debugger, &device->debugger.show_debugger, device->get_device_descriptor());
		}
	}

	if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, height, window_flags)) 
	{
		if (ImGui::BeginMenuBar()) {
			fds_rom* fds = reinterpret_cast<fds_rom*>(denver->mainbus->find_device_partial_name_match("Denver FDS Hardware"));
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open file", "Ctrl+O")) {
					std::cout << "Last opened path: " << state->lastpath << "\n";
					ImGuiFileDialog::Instance()->OpenDialog("nesfile", "Select NES file", "All compatible files{.nes,.nsf,.nsfe,.fds},NES roms{.nes},NES FDS Images{.fds},NES music{.nsf},NES extended music{.nsfe}", state->lastpath, 1, nullptr, ImGuiFileDialogFlags_Modal);
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
				if (ImGui::BeginMenu("Shaders")) {
					for (std::string shader : shaderList) {
						if (ImGui::MenuItem(shader.c_str(), NULL, false)) {
							// apply shader.
							denver->use_shader(shader.c_str());
						}
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Audio options")) {
					if (ImGui::MenuItem("Enable audio harmonics filtering", NULL, denver->audio->interpolated)) {
						denver->audio->interpolated = !denver->audio->interpolated;
					}
					if (ImGui::MenuItem("Use HQ filter", NULL, denver->audio->hq_filter)) {
						denver->audio->hq_filter= !denver->audio->hq_filter;
					}
					if (ImGui::MenuItem("Use highres stepping / linear mixing (ideal chips)", NULL, denver->audio->high_res_linear_mix)) {
						denver->audio->high_res_linear_mix = !denver->audio->high_res_linear_mix;
						denver->audio->update_devices();
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
				// volume bar.
				if (ImGui::BeginMenu("Volume")) {
					if (ImGui::MenuItem("Unlock 2x volume setting for audio devices", NULL, state->unlock20)) {
						state->unlock20 = !state->unlock20;
					}
					float maxVol = state->unlock20 ? 2.0f : 1.0f;					
					ImGui::SliderFloat("Main vol", &denver->audio->main_volume, 0.1f, 1.0f, "%.1f", 0);
					ImGui::Separator();
					for (auto device : denver->audio->audibles) {
						ImGui::SliderFloat(device->get_device_descriptor(), &device->device_volume, 0.1f, maxVol, "%.1f", 0);
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (fds) {
				if (ImGui::BeginMenu("FDS")) {
					if (ImGui::BeginMenu("Swap disk")) {
						int disks = (int)fds->disks.size();
						for (int i = 0; i < disks; i++) {
							int sideab = i % 2;
							int diskno = i / 2;
							diskno++;
							std::string descriptor = "Disk #" + std::to_string(diskno);
							if (sideab == 0) {
								descriptor += " side A";
							}
							else {
								descriptor += " side B";
							}
							if (ImGui::MenuItem(descriptor.c_str())) {
								// select the disk.
								fds->set_side(i);
							}
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Disk actions")) {
						if (ImGui::MenuItem("Eject disk", NULL, false)) {
							if (fds) {
								fds->disk_inserted = false;
							}

						}
						if (ImGui::MenuItem("Insert disk", NULL, false)) {
							if (fds) {
								fds->disk_inserted = true;
								fds->state.disksector = 0;
								fds->state.cyclecount = 0;
							}
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
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
				if (ImGui::MenuItem("Pause emulation", "Ctrl+P", denver->clock.pause)) {
					denver->clock.pause = !denver->clock.pause;
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
								device->debugger.show_debugger = true;
							}
						}
						ImGui::SeparatorText("PPU BUS");
						for (auto device : denver->ppu_device->vbus.devices) {
							if (ImGui::MenuItem(device->get_device_descriptor())) {
								device->debugger.show_debugger = true;
							}
						}
						ImGui::SeparatorText("Audio BUS");
						for (auto device : denver->audio->audibles) {
							if (ImGui::MenuItem(device->get_device_descriptor())) {
								device->debugger.show_debugger = true;
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
	}
	ImGui::End();

	ImVec2 startRendering = ImGui::GetMainViewport()->Pos;
	ImVec2 sizeRendering = ImGui::GetMainViewport()->WorkSize;
	startRendering.y += height;
	ImGui::SetNextWindowPos(startRendering);
	ImGui::SetNextWindowSize(sizeRendering);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	if (denver->cart->nsf_mode) {
		if (ImGui::Begin("NES Music Player", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			// NSF interface.
			ImGui::Text("Denver NSF Player");
			ImGui::NewLine();
			ImGui::Text("Song(s) :");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4{1.0f, 1.0f, 0.0f, 1.0f}, "%s", denver->cart->songname.c_str());
			ImGui::Text("Artist :");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "%s", denver->cart->artist.c_str());
			ImGui::Text("Copyright :");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "%s", denver->cart->copyright.c_str());
			ImGui::Text("Ripper :");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "%s", denver->cart->ripper.c_str());
			ImGui::NewLine();
			ImGui::Separator();

			// get NSF interface.
			nsfrom* nsfinterface = reinterpret_cast<nsfrom*>(denver->cart->program);
			state->zeroIndexedTrackNo = nsfinterface->state.currentsong - 1;

			ImGui::Text("Selected song %d/%d", nsfinterface->state.currentsong, nsfinterface->state.numsongs);

			if (denver->cart->trackNames.size() > 0) {
				ImGui::Text("Track: "); ImGui::SameLine();
				ImGui::TextColored(ImVec4{ 0.0f, 1.0f, 1.0f, 1.0f }, "%s", denver->cart->trackNames[nsfinterface->state.currentsong - 1].c_str());
			}

			ImGui::Separator();

			ImGui::Text("Select track:");
			ImGui::SameLine();
			if (denver->cart->trackNames.size() > 0) {
				// tracknamed selection.
				std::vector<const char*>cItems;
				cItems.clear();
				for (std::string &track : denver->cart->trackNames) {
					cItems.push_back(track.c_str());
				}
				if (ImGui::Combo("##", &state->zeroIndexedTrackNo, cItems.data(), nsfinterface->state.numsongs)) {
					nsfinterface->initialize(state->zeroIndexedTrackNo);
					nsfinterface->state.currentsong = state->zeroIndexedTrackNo + 1;
				}
			}
			else {
				// Track 1., Track 2., etc selection.
				std::vector<const char*>cItems;
				std::vector<std::string>names;
				names.reserve(nsfinterface->state.numsongs + 1);
				cItems.clear();
				for (int i = 0; i < nsfinterface->state.numsongs; i++) {
					std::string track = std::string("Track ") + std::to_string(i + 1) + std::string(".");
					names.push_back(track);
					cItems.push_back(names.back().c_str());
				}
				if (ImGui::Combo("##", &state->zeroIndexedTrackNo, cItems.data(), nsfinterface->state.numsongs)) {
					nsfinterface->initialize(state->zeroIndexedTrackNo);
					nsfinterface->state.currentsong = state->zeroIndexedTrackNo + 1;
				}
			}
	
			ImGui::Separator();

			// Media buttons.
			if (ImGui::Button("<<", ImVec2{ 128, 24 * state->scaling })) {
				if (nsfinterface->state.currentsong > 1)
					nsfinterface->state.currentsong--;
				// play the track.
				nsfinterface->initialize(nsfinterface->state.currentsong - 1);
			}
			ImGui::SameLine();
			ImGui::Text("  ");
			ImGui::SameLine();
			if (!denver->clock.pause) {
				if (ImGui::Button("||", ImVec2{ 128, 24 * state->scaling })) {
					denver->clock.pause = !denver->clock.pause;
				}
			}
			else {
				if (ImGui::Button("|>", ImVec2{ 128, 24 * state->scaling })) {
					denver->clock.pause = !denver->clock.pause;
				}
			}
			ImGui::SameLine();
			ImGui::Text("  ");
			ImGui::SameLine();
			if (ImGui::Button(">>", ImVec2{ 128, 24 * state->scaling })) {
				if (nsfinterface->state.currentsong < nsfinterface->state.numsongs)
					nsfinterface->state.currentsong++;
				nsfinterface->initialize(nsfinterface->state.currentsong - 1);
			}
			ImGui::SameLine();
			ImGui::Text("  ");
			ImGui::SameLine();
			if (ImGui::Button(">", ImVec2{ 128, 24 * state->scaling })) {
				nsfinterface->initialize(nsfinterface->state.currentsong - 1);
			}
			ImGui::Separator();
			ImGui::Text("Live Sample output");
			int s = (int)denver->audio->final_mux.size();
			float* graph = &denver->audio->final_mux[0];
			float lb = denver->audio->average_mix - 0.7f;
			float hb = denver->audio->average_mix + 0.7f;			
			ImGui::PlotLines("Sample", graph, s, 0, NULL, lb, hb, ImVec2{ 0, 160.0f * state->scaling });
			ImGui::Separator();
			ImGui::Text("Audio devices: (check to mute)");
			ImGui::Checkbox("NES 2A03 APU", &denver->nes_2a03->apu_2a03.muted);
			if (denver->cart->fdsexp) {
				ImGui::Checkbox("Nintendo FDS", &denver->cart->fdsexp->muted);
			}
			if (denver->cart->namexp) {
				ImGui::Checkbox("Namco", &denver->cart->namexp->muted);
			}
			if (denver->cart->vrc6exp) {
				ImGui::Checkbox("Konami VRC6", &denver->cart->vrc6exp->muted);
			}
			if (denver->cart->vrc7exp) {
				ImGui::Checkbox("Konami VRC7", &denver->cart->vrc7exp->muted);
			}
			if (denver->cart->sunexp) {
				ImGui::Checkbox("Sunsoft", &denver->cart->sunexp->muted);
			}
			if (denver->cart->mmc5exp) {
				ImGui::Checkbox("Nintendo MMC", &denver->cart->mmc5exp->muted);
			}
			if (denver->cart->high_hz_nsf) {
				ImGui::Text("High refresh mode (3x CPU speed)"); 
			}
			ImGui::Separator();
			// compute time elapsed.
			//uint64_t currenttime = SDL_GetTicks64();
			uint32_t elapsed = (uint32_t)nsfinterface->return_time_in_ms();
			uint32_t total = 0;
			uint32_t seconds = elapsed / 1000;
			uint32_t minutes = seconds / 60;
			seconds = seconds % 60;
			uint32_t hseconds = elapsed - ((minutes * 60) + seconds) * 1000;
			hseconds /= 10;
			ImGui::Text("Time elapsed: %02d:%02d:%02d", minutes, seconds, hseconds); ImGui::SameLine();
			// check if lengths are defined???
			if (denver->cart->trackLengths.size() != 0) {
				if (nsfinterface->state.currentsong - 1 <= denver->cart->trackLengths.size()) {
					// we know the length of the track add it to the line.					
					elapsed = denver->cart->trackLengths[nsfinterface->state.currentsong - 1];
					seconds = elapsed / 1000;
					minutes = seconds / 60;
					seconds = seconds % 60;
					hseconds = elapsed - ((minutes * 60) + seconds) * 1000;
					hseconds /= 10;
					ImGui::Text("/"); ImGui::SameLine();
					ImGui::Text("%02d:%02d:%02d", minutes, seconds, hseconds);
				}
				else ImGui::NewLine();
			}
			ImGui::NewLine();
			// progress bar when tracklength is known.
			if (denver->cart->trackLengths.size() != 0) {
				total = denver->cart->trackLengths[nsfinterface->state.currentsong - 1];
				elapsed = (uint32_t)nsfinterface->return_time_in_ms();
				int w, h;
				SDL_GetWindowSize(state->mainwin, &w, &h);
				w -= 32;
				float prog = (1.0f / (float)total) * (float)elapsed;
				ImGui::ProgressBar(prog, ImVec2{ (float)w, 24 * state->scaling });
			}
			// repeat track after 5s.
			if (denver->cart->trackLengths.size() != 0) {
				ImGui::Checkbox("Repeat track @ End Track + extra 5 seconds.", &state->repeatTrackAfterEnd5s);
				if (state->repeatTrackAfterEnd5s) {
					total = denver->cart->trackLengths[nsfinterface->state.currentsong - 1];
					elapsed = (uint32_t)nsfinterface->return_time_in_ms();
					if (elapsed > total + 5000) {
						nsfinterface->initialize(nsfinterface->state.currentsong - 1);
					}
				}
			}

		}
		ImGui::End();
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
	}
	ImGui::End();

}