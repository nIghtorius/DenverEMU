#include "gui_debuggers.h"
#include "imgui.h"
#include <string>

#pragma warning(disable : 4996)

// implementation

void	denvergui::render_apuviewer(nes_emulator *denver, denvergui_state *state) {
	if (ImGui::Begin("APU debugger", &state->show_apu_debugger, 0)) {
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
			ImGui::PlotLines("Sample", graph, s, 0, NULL, lb, hb, ImVec2{ 0, 160.0f });
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Audio Mixing Specifics")) {
			ImGui::Text("Attentuation: %f", (1.0f - denver->audio->attentuate));
			ImGui::Text("Number of audio devices: %d", (denver->audio->audibles.size()));
		}
		ImGui::End();
	}
}

void	denvergui::render_cpuviewer(nes_emulator *denver, denvergui_state *state) {
	if (ImGui::Begin("CPU debugger", &state->show_cpu_debugger)) {
		if (ImGui::TreeNode("CPU registers")) {
			ImGui::Text("pc = %02X, sp = %02X, sr = %02X", denver->nes_2a03->cpu_2a03.regs.pc,
				denver->nes_2a03->cpu_2a03.regs.sp, denver->nes_2a03->cpu_2a03.regs.sr);
			ImGui::Text("ac = %02X, x = %02X, y = %02X", denver->nes_2a03->cpu_2a03.regs.ac,
				denver->nes_2a03->cpu_2a03.regs.x, denver->nes_2a03->cpu_2a03.regs.y);
			ImGui::Text("Status Flags: %s%s%s%s%s%s%s",
				(denver->nes_2a03->cpu_2a03.regs.sr & cf_negative) > 0 ? "N" : "_",
				(denver->nes_2a03->cpu_2a03.regs.sr & cf_overflow) > 0 ? "O" : "_",
				(denver->nes_2a03->cpu_2a03.regs.sr & cf_break) > 0 ? "B" : "_",
				(denver->nes_2a03->cpu_2a03.regs.sr & cf_decimal) > 0 ? "D" : "_",
				(denver->nes_2a03->cpu_2a03.regs.sr & cf_interrupt) > 0 ? "I" : "_",
				(denver->nes_2a03->cpu_2a03.regs.sr & cf_zero) > 0 ? "Z" : "_",
				(denver->nes_2a03->cpu_2a03.regs.sr & cf_carry) > 0 ? "C" : "_");
			ImGui::Text("Reset: %04X, NMI: %04X, IRQ: %04X",
				denver->mainbus->readmemory_as_word(vector_reset, true),
				denver->mainbus->readmemory_as_word(vector_nmi, true),
				denver->mainbus->readmemory_as_word(vector_irq, true)
				);
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Disassembler")) {
			ImGui::Text("Set disassemble address:");
			ImGui::InputInt("ADDRESS:", &state->disasm_start, 1, 100, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
			if (ImGui::Button("GO RESETVECTOR")) {
				state->disasm_start = denver->mainbus->readmemory_as_word(vector_reset, true);
			}
			ImGui::SameLine();
			if (ImGui::Button("GO NMIVECTOR")) {
				state->disasm_start = denver->mainbus->readmemory_as_word(vector_nmi, true);
			}
			ImGui::SameLine();
			if (ImGui::Button("GO IRQVECTOR")) {
				state->disasm_start = denver->mainbus->readmemory_as_word(vector_irq, true);
			}
			ImGui::SameLine();
			if (ImGui::Button("GO CPU,r.PC")) {
				state->disasm_start = denver->nes_2a03->cpu_2a03.regs.pc;
			}
			ImGui::Text("Disassembling from: 0x%04X", state->disasm_start);
			if (ImGui::BeginTable("Disassembly", 3, ImGuiTableFlags_SizingFixedSame)) {
				ImGui::TableSetupColumn("ADDRESS", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("DATA", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("DISASSEMBLY", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableHeadersRow();
			
				// prepare the disassembler.
				int disaddr = state->disasm_start;
				disassembler *dasm = new disassembler();
				dasm->set_address(disaddr);
				dasm->set_mainbus(denver->mainbus);

				// disassemble up to 40 lines.
				for (int i = 0; i < 40; i++) {
					std::string distxt = dasm->disassemble();
					int size = dasm->last_instruction_size;
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);	// ADDRESS
					ImGui::Text("0x%04X", disaddr);
					ImGui::TableSetColumnIndex(1);	// DATA
					switch (size) {
					case 1:
						ImGui::Text("%02X", (int)denver->mainbus->readmemory(disaddr, true));
						disaddr++;
						break;
					case 2:
						ImGui::Text("%02X %02X", (int)denver->mainbus->readmemory(disaddr, true),
							(int)denver->mainbus->readmemory(disaddr + 1), true);
						disaddr += 2;
						break;
					case 3:
						ImGui::Text("%02X %02X %02X", (int)denver->mainbus->readmemory(disaddr, true),
							(int)denver->mainbus->readmemory(disaddr + 1, true),
							(int)denver->mainbus->readmemory(disaddr + 2), true);
						disaddr += 3;
						break;
					}
					ImGui::TableSetColumnIndex(2); // DISASSEMBLY
					ImGui::Text(distxt.c_str());
				}
				ImGui::EndTable();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Stack")) {
			// show as 16x16 (256 byte) view.
			int stack = 0x0100;
			ImGui::Text("STACK:");
			for (int i = 0; i < 16; i++) {
				ImGui::Text("0x%04X   ", stack);
				ImGui::SameLine();
				for (int j = 0; j < 16; j++) {
					byte sb = denver->mainbus->readmemory(stack++, true);
					ImGui::Text("%02X ", (int)sb);
					if (j != 15) ImGui::SameLine();
				}
			}
			ImGui::Text("");
			ImGui::Text("WORD @ SP: %04X  --  BYTE @ SP: %02X",
				denver->mainbus->readmemory_as_word(0x0100 + denver->nes_2a03->cpu_2a03.regs.sp, true),
				denver->mainbus->readmemory(0x0100 + denver->nes_2a03->cpu_2a03.regs.sp, true)
				);
			ImGui::Separator();
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

void	denvergui::render_ppuviewer(nes_emulator *denver, denvergui_state *state) {
	if (ImGui::Begin("PPU viewer", &state->show_ppu_debugger)) {
		if (ImGui::TreeNode("Basics")) {
			ImGui::Text("PPU basics");
			ImGui::InputInt("CAPTURE CYCLE", &denver->ppu_device->capture_cycle, 1, 100, ImGuiInputTextFlags_CharsDecimal);
			if (ImGui::BeginTable("EMULATION PARAMETERS", 2, ImGuiTableFlags_SizingFixedFit)) {
				ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableHeadersRow();

				// show values.
				ImGui::TableNextRow();
				
				// latch
				ImGui::TableSetColumnIndex(0);	ImGui::Text("Latch");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_latch);

				// p2007buffer
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("Port 2007 buffer");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%02X", denver->ppu_device->dbg_p2007buf);

				// dbg_ppuaddr
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("PPU ADDR");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%04X", denver->ppu_device->dbg_ppuaddr);

				// dbg_cycle
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("DOT_CYCLE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_cycle);

				// dbg_sl
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("Scanline");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_sl);

				// dbg_beam
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("Beam");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_beam);

				// base name table
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("BASE_NAME_TABLE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%02X", denver->ppu_device->dbg_ppuctrl.base_name_table);
				// increment 32 bytes.
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("INC_32_BYTES");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuctrl.increment_32_bytes ? "True" : "False");
				// sprites @ 0x1000
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITES_AT_0X1000");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuctrl.sprites_0x1000 ? "True" : "False");
				// bg @ 0x1000
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("BACKGROUND_AT_0X1000");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuctrl.bg_0x1000 ? "True" : "False");
				// 8x16 sprites?
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITES_8X16");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuctrl.sprites_8x16 ? "True" : "False");
				// master_mode
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("MASTERMODE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuctrl.master_mode ? "True" : "False");
				// NMI
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("TRIGGER_NMI");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuctrl.do_nmi ? "True" : "False");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("GRAYSCALE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppumask.grayscale ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SHOW_LEFT_8PX_BG");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppumask.bg8lt ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SHOW_LEFT_8PX_SPR");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppumask.spr8lt ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("BG_ENABLE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppumask.showbg ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPR_ENABLE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppumask.showspr ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("EMPHASIS_RED");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppumask.emp_red ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("EMPHASIS_GREEN");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppumask.emp_grn ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("EMPHASIS_BLUE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppumask.emp_blu ? "True" : "False");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_OVERFLOW");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppustatus.sprite_overflow ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_0_HIT");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppustatus.sprite_0_hit ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("VBLANK");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppustatus.vblank ? "True" : "False");

				// ppu_internal. big one.
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("ADDRESS WRITE LATCH");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuint.address_write_latch ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("FINE X SHIFT");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_ppuint.x_shift);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("FINE Y SHIFT");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_ppuint.y_shift);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("T_REGISTER (TEMP)");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%04X", denver->ppu_device->dbg_ppuint.t_register);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("V_REGISTER");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%04X", denver->ppu_device->dbg_ppuint.v_register);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("PATTERN_SHIFT_REG_LO");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%04X", denver->ppu_device->dbg_ppuint.shiftregs_pattern[0]);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("PATTERN_SHIFT_REG_HI");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%04X", denver->ppu_device->dbg_ppuint.shiftregs_pattern[1]);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("ATTRIBUTE_SHIFT_REG_LO");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%04X", denver->ppu_device->dbg_ppuint.shiftreg_attribute[0]);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("ATTRIBUTE_SHIFT_REG_HI");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%04X", denver->ppu_device->dbg_ppuint.shiftreg_attribute[1]);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SECONDARY OAM");
				ImGui::TableSetColumnIndex(1);	
				for (int i = 0; i < 8; i++) {
					ImGui::Text("%d - %02X %02X %02X %02X", i, denver->ppu_device->dbg_ppuint.secoam[i].y,
						denver->ppu_device->dbg_ppuint.secoam[i].tile,
						denver->ppu_device->dbg_ppuint.secoam[i].attr,
						denver->ppu_device->dbg_ppuint.secoam[i].x);
					if (!(i % 2)) ImGui::SameLine();
				}
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_EVAL_N_IDX");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_ppuint.n);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_EVAL_OAM_CPY_M_CTR");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_ppuint.m);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_EVAL_OAM_CPY_SN");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%d", denver->ppu_device->dbg_ppuint.sn);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SECOAM CLEAR PHASE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuint.oam_clearing ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SECOAM COPY PHASE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuint.oam_copy ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SECOAM EVAL COMPLETE");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuint.oam_evald ? "True" : "False");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_PATTERN_LO(8X)");
				ImGui::TableSetColumnIndex(1);
				for (int i = 0; i < 8; i++) {
					ImGui::Text("%02X", denver->ppu_device->dbg_ppuint.shiftreg_spr_pattern_lo[i]);
					ImGui::SameLine();
				}
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_PATTERN_HI(8X)");
				ImGui::TableSetColumnIndex(1);
				for (int i = 0; i < 8; i++) {
					ImGui::Text("%02X", denver->ppu_device->dbg_ppuint.shiftreg_spr_pattern_hi[i]);
					ImGui::SameLine();
				}
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_LATCH(ATTR)(8X)");
				ImGui::TableSetColumnIndex(1);
				for (int i = 0; i < 8; i++) {
					ImGui::Text("%02X", denver->ppu_device->dbg_ppuint.shiftreg_spr_latch[i]);
					ImGui::SameLine();
				}
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("SPRITE_CTR(8X)");
				ImGui::TableSetColumnIndex(1);
				for (int i = 0; i < 8; i++) {
					ImGui::Text("%02X", denver->ppu_device->dbg_ppuint.shiftreg_spr_counter[i]);
					ImGui::SameLine();
				}
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);	ImGui::Text("ODD FRAME");
				ImGui::TableSetColumnIndex(1);	ImGui::Text("%s", denver->ppu_device->dbg_ppuint.odd_even_frame ? "True" : "False");
				ImGui::EndTable();
			}

			// PPU_INTERNALS.
			if (ImGui::BeginTable("PPU INTERNALS", 2, ImGuiTableFlags_SizingFixedFit)) {
				ImGui::EndTable();
			}

			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Pattern Table")) {
			// palette chooser.
			const char * items[] = { "BG PAL#0", "BG PAL#1", "BG PAL#2", "BG PAL#3",
								"SPR PAL#0", "SPR PAL#1", "SPR PAL#2", "SPR PAL#3" };
			ImGui::Text("Select active palette");
			ImGui::SameLine();
			ImGui::Combo("palette", &state->pattern_palette, items, 8);
			// render the pattern table.
			ImGui::Image((void *)(intptr_t)state->pattern_tex, ImVec2 { 512, 256 });
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Name Table")) {
			ImGui::Checkbox("Show scroll registers", &state->show_scroll_regs);
			ImGui::Checkbox("Show PPU updates from cpu", &state->show_ppu_updates);
			// render the name table.
			ImGui::Image((void *)(intptr_t)state->ntable_tex, ImVec2 { 512, 480 });
			ImGui::Separator();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("PPU Updates during rendering")) {
			int count = 1;
			if (ImGui::BeginTable("PPU Updates", 8, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders)) {
				ImGui::TableSetupColumn("event#", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("ppuaddr", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("ppucycle", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("cpucycle", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("beam(x)", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("scanline(y)", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("data", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("command", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableHeadersRow();

				// show values.
				ImGui::TableNextRow();

				for (ppu_cpu_event& event : denver->ppu_device->events) {
					ImGui::TableSetColumnIndex(0);	ImGui::Text("%d", count);
					ImGui::TableSetColumnIndex(1);	ImGui::Text("%04X", event.ppuaddr);
					ImGui::TableSetColumnIndex(2);	ImGui::Text("%d", event.ppucycle);
					ImGui::TableSetColumnIndex(3);	ImGui::Text("%d", event.cpucycle);
					ImGui::TableSetColumnIndex(4);	ImGui::Text("%d", event.beam);
					ImGui::TableSetColumnIndex(5);	ImGui::Text("%d", event.scanline);
					ImGui::TableSetColumnIndex(6);	ImGui::Text("%02X", event.data);
					ImGui::TableSetColumnIndex(7);

					switch (event.ppuaddr & 0xFF) {
					case PPU_PPUCTRL_PORT:
						ImGui::Text("CTRL_PORT");
						break;
					case PPU_PPUMASK_PORT:
						ImGui::Text("MASK_PORT");
						break;
					case PPU_OAMADDR_PORT:
						ImGui::Text("OAMADDR_PORT");
						break;
					case PPU_SCROLL_PORT:
						ImGui::Text("SCROLL_PORT");
						break;
					case PPU_ADDRESS_PORT:
						ImGui::Text("ADDRESS_PORT");
						break;
					default:
						ImGui::Text("INVALID");
						break;
					}

					ImGui::TableNextRow();
					count++;
				}

				ImGui::EndTable();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}
		ImGui::End();
	}
}