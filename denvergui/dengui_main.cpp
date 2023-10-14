#include "dengui_main.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "../imguifiledialog/ImGuiFileDialog.h"
#include "../package/2a03.h"
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
			std::cout << "ImGuiFileDialog: " << ImGuiFileDialog::Instance()->GetFilePathName() << std::endl;
			// load the cart.
			denver->load_cartridge(ImGuiFileDialog::Instance()->GetFilePathName().c_str());
			denver->cold_reset();
		}
		ImGuiFileDialog::Instance()->Close();
	}

	// apu dialog.
	if (state->show_apu_debugger) {
		if (ImGui::Begin("APU debugger", &state->show_apu_debugger, 0)) {
			//ImGui::BeginTabBar("##aputabs");
			if (ImGui::TreeNode("Pulse #1")) {
				char buf[16];
				ImGui::Text("Enabled: %s", (denver->nes_2a03->apu_2a03.pulse[0].enabled ? "Yes" : "No"));
				ImGui::Text("Duty cycle: %d", denver->nes_2a03->apu_2a03.pulse[0].duty_cycle);
				ImGui::Text("Dyte position: %d", denver->nes_2a03->apu_2a03.pulse[0].duty_pos);
				ImGui::Text("Envelope loop: %s", (denver->nes_2a03->apu_2a03.pulse[0].envelope_loop ? "Yes" : "No"));
				ImGui::Text("Contant volume: %s", (denver->nes_2a03->apu_2a03.pulse[0].constant_volume ? "Yes" : "No"));
				ImGui::Text("Volume / Envelope");
				float vol = (1.0f / 15.0f)*(float)denver->nes_2a03->apu_2a03.pulse[0].volume_envelope;
				sprintf(buf, "%d/15", denver->nes_2a03->apu_2a03.pulse[0].volume_envelope);
				ImGui::ProgressBar(vol, ImVec2{ 0, 0 }, buf);
				ImGui::Text("Timer: %d", denver->nes_2a03->apu_2a03.pulse[0].timer);
				ImGui::Text("Length Counter: %d", denver->nes_2a03->apu_2a03.pulse[0].length_counter);
				ImGui::Text("Sweeping: %s", (denver->nes_2a03->apu_2a03.pulse[0].sweep_enable ? "Yes" : "No"));
				ImGui::Text("Sweep divider: %d", denver->nes_2a03->apu_2a03.pulse[0].sweep_divider);
				ImGui::Text("Sweep negation: %s", (denver->nes_2a03->apu_2a03.pulse[0].sweep_negate ? "Yes" : "No"));
				ImGui::Text("Envelope Count: %d", denver->nes_2a03->apu_2a03.pulse[0].envelope_count);
				ImGui::Text("Envelope Out");
				float envout = (1.0f / 15.0f)*(float)denver->nes_2a03->apu_2a03.pulse[0].envelope_out;
				sprintf(buf, "%d/15", denver->nes_2a03->apu_2a03.pulse[0].envelope_out);
				ImGui::ProgressBar(envout, ImVec2{ 0, 0 }, buf);
				ImGui::Separator();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Pulse #2")) {
				char buf[16];
				ImGui::Text("Enabled: %s", (denver->nes_2a03->apu_2a03.pulse[1].enabled ? "Yes" : "No"));
				ImGui::Text("Duty cycle: %d", denver->nes_2a03->apu_2a03.pulse[1].duty_cycle);
				ImGui::Text("Dyte position: %d", denver->nes_2a03->apu_2a03.pulse[1].duty_pos);
				ImGui::Text("Envelope loop: %s", (denver->nes_2a03->apu_2a03.pulse[1].envelope_loop ? "Yes" : "No"));
				ImGui::Text("Contant volume: %s", (denver->nes_2a03->apu_2a03.pulse[1].constant_volume ? "Yes" : "No"));
				ImGui::Text("Volume / Envelope");
				float vol = (1.0f / 15.0f)*(float)denver->nes_2a03->apu_2a03.pulse[1].volume_envelope;
				sprintf(buf, "%d/15", denver->nes_2a03->apu_2a03.pulse[1].volume_envelope);
				ImGui::ProgressBar(vol, ImVec2{ 0, 0 }, buf);
				ImGui::Text("Timer: %d", denver->nes_2a03->apu_2a03.pulse[1].timer);
				ImGui::Text("Length Counter: %d", denver->nes_2a03->apu_2a03.pulse[1].length_counter);
				ImGui::Text("Sweeping: %s", (denver->nes_2a03->apu_2a03.pulse[1].sweep_enable ? "Yes" : "No"));
				ImGui::Text("Sweep divider: %d", denver->nes_2a03->apu_2a03.pulse[1].sweep_divider);
				ImGui::Text("Sweep negation: %s", (denver->nes_2a03->apu_2a03.pulse[1].sweep_negate ? "Yes" : "No"));
				ImGui::Text("Envelope Count: %d", denver->nes_2a03->apu_2a03.pulse[1].envelope_count);
				ImGui::Text("Envelope Out");
				float envout = (1.0f / 15.0f)*(float)denver->nes_2a03->apu_2a03.pulse[1].envelope_out;
				sprintf(buf, "%d/15", denver->nes_2a03->apu_2a03.pulse[1].envelope_out);
				ImGui::ProgressBar(envout, ImVec2{ 0, 0 }, buf);
				ImGui::Separator();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Triangle")) {
				ImGui::Text("Enabled: %s", (denver->nes_2a03->apu_2a03.triangle.enabled ? "Yes" : "No"));
				ImGui::Text("Triangle Length Loop: %d", denver->nes_2a03->apu_2a03.triangle.triangle_length);
				ImGui::Text("Triangle Length Counter: %d", denver->nes_2a03->apu_2a03.triangle.triangle_length_counter);
				ImGui::Text("Length Counter: %d", denver->nes_2a03->apu_2a03.triangle.length_counter);
				ImGui::Text("Triangle Length Loop: %s", (denver->nes_2a03->apu_2a03.triangle.triangle_length_loop ? "Yes" : "No"));
				ImGui::Text("Triangle Length Reload: %s", (denver->nes_2a03->apu_2a03.triangle.triangle_counter_reload ? "Yes" : "No"));
				ImGui::Text("Timer: %d", denver->nes_2a03->apu_2a03.triangle.timer);
				ImGui::Text("Sequencer: %d", denver->nes_2a03->apu_2a03.triangle.sequencer);
				ImGui::Separator();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Noise")) {
				char buf[16];
				ImGui::Text("Enabled: %s", (denver->nes_2a03->apu_2a03.noise.enabled ? "Yes" : "No"));
				ImGui::Text("Envelope loop: %s", (denver->nes_2a03->apu_2a03.noise.envelope_loop ? "Yes" : "No"));
				ImGui::Text("Constant Volume: %s", (denver->nes_2a03->apu_2a03.noise.constant_volume ? "Yes" : "No"));
				ImGui::Text("Volume Envelope");
				float vol = (1.0f / 15.0f)*(float)denver->nes_2a03->apu_2a03.noise.volume_envelope;
				sprintf(buf, "%d/15", denver->nes_2a03->apu_2a03.noise.volume_envelope);
				ImGui::ProgressBar(vol, ImVec2{ 0, 0 }, buf);
				ImGui::Text("Envelope Out");
				float envvol = (1.0f / 15.0f)*(float)denver->nes_2a03->apu_2a03.noise.envelope_out;
				sprintf(buf, "%d/15", denver->nes_2a03->apu_2a03.noise.envelope_out);
				ImGui::ProgressBar(envvol, ImVec2{ 0, 0 }, buf);
				ImGui::Text("Noise loop: %s", (denver->nes_2a03->apu_2a03.noise.noise_loop ? "Yes" : "No"));
				ImGui::Text("Noise period: %d", denver->nes_2a03->apu_2a03.noise.noise_period);
				ImGui::Text("Length Counter: %d", denver->nes_2a03->apu_2a03.noise.length_counter);
				ImGui::Text("Noise shift register: %d", denver->nes_2a03->apu_2a03.noise.noise_shift_reg);
				ImGui::Separator();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("DMC")) {
				ImGui::Text("Enabled: %s", (denver->nes_2a03->apu_2a03.dmc.enabled ? "Yes" : "No"));
				ImGui::Text("Asserting IRQ: %s", (denver->nes_2a03->apu_2a03.dmc.irq_asserted ? "Yes" : "No"));
				ImGui::Text("IRQ Enabled: %s", (denver->nes_2a03->apu_2a03.dmc.irq_enable ? "Yes" : "No"));
				ImGui::Text("Rate: %d", denver->nes_2a03->apu_2a03.dmc.rate);
				ImGui::Text("Output level: %d", denver->nes_2a03->apu_2a03.dmc.direct_out);
				ImGui::Text("Sample address: %#04X", denver->nes_2a03->apu_2a03.dmc.sample_addr);
				ImGui::Text("Sample length: %d", denver->nes_2a03->apu_2a03.dmc.sample_length_load);
				ImGui::Text("Sample buffer: %d", denver->nes_2a03->apu_2a03.dmc.sample_buffer);
				ImGui::Text("Silent: %s", (denver->nes_2a03->apu_2a03.dmc.silent ? "Yes" : "No"));
				ImGui::Text("Sample Shift Register: %d", denver->nes_2a03->apu_2a03.dmc.sample_shift_register);
				ImGui::Separator();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Live Sample Data")) {
				int s = (int)denver->audio->final_mux.size();
				float * graph = &denver->audio->final_mux[0];		
				float lb = denver->audio->average_mix - 0.7f;
				float hb = denver->audio->average_mix + 0.7f;
				ImGui::PlotLines("Sample", graph, s, 0, NULL, lb, hb, ImVec2{0, 160.0f});
				ImGui::Separator();
				ImGui::TreePop();
			}
			//ImGui::EndTabBar();
			ImGui::End();
		}
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
				if (ImGui::BeginMenu("[dbg]Emulation cycle sync")) {
					if (ImGui::MenuItem("Sync as fast as possible (slow)")) {
						denver->clock.set_sync_cycle_in_ppucycles(1);
					}
					if (ImGui::MenuItem("Sync around ~35 PPU cycles")) {
						denver->clock.set_sync_cycle_in_ppucycles(35);
					}
					if (ImGui::MenuItem("Sync around ~70 PPU cycles")) {
						denver->clock.set_sync_cycle_in_ppucycles(70);
					}
					if (ImGui::MenuItem("Sync around ~128 PPU cycles (fastest)")) {
						denver->clock.set_sync_cycle_in_ppucycles(128);
					}
					ImGui::EndMenu();
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
	if (ImGui::Begin("NES Game", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize || ImGuiWindowFlags_NoInputs)) 
	{
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
		ImGui::Image((void *)(intptr_t)tex, ImVec2{ width_x, height_y });
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
		ImGui::End();
	}

}