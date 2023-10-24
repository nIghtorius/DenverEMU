#pragma once

#include "../emulator/nes.h"

namespace denvergui {
	struct denvergui_state {
		// dialog states.
		bool show_apu_debugger = false;
		bool show_cpu_debugger = false;
		bool show_ppu_debugger = false;

		bool apu_tab_pulse1;
		bool apu_tab_pulse2;
		bool apu_tab_tri;
		bool apu_tab_noi;
		bool apu_tab_dmc;
		
		// debugging.
		bool write_exec_log = false;

		// cpu viewer.
		int	disasm_start = 0x8000;
	};

	void	render_main (nes_emulator *denver,  GLuint tex, denvergui_state *state);
};

