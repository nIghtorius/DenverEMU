#include "dengui_main.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "../imguifiledialog/ImGuiFileDialog.h"
#include <iostream>

void	denvergui::render_main (nes_emulator *denver, GLuint tex) {
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

	//if (ImGui::BeginViewportSideBar("##NES", viewport, ImGuiDir_Left, io.DisplaySize.y - height * 2, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) 
	ImVec2 startRendering = ImGui::GetMainViewport()->Pos;
	ImVec2 sizeRendering = ImGui::GetMainViewport()->WorkSize;
	startRendering.y += height;
	ImGui::SetNextWindowPos(startRendering);
	ImGui::SetNextWindowSize(sizeRendering);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	if (ImGui::Begin("##NES", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize)) 
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
	ImGui::PopStyleVar(2);

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