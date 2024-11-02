#pragma once

#include "../emulator/nes.h"

namespace denvergui {
	struct denvergui_state {
		// dialog states.
		bool show_apu_debugger = false;
		bool show_cpu_debugger = false;
		bool show_ppu_debugger = false;

		bool apu_tab_pulse1 = false;
		bool apu_tab_pulse2 = false;
		bool apu_tab_tri = false;
		bool apu_tab_noi = false;
		bool apu_tab_dmc = false;
		
		// debugging.
		bool write_exec_log = false;

		// cpu viewer.
		int	disasm_start = 0x8000;
		bool lock_disasm_cpu = false;

		// ppu palette (pattern)
		int	pattern_palette = 0x00;
		GLuint	pattern_tex = 0;
		GLuint  ntable_tex = 0;
		bool	show_scroll_regs = false;
		bool	show_ppu_updates = false;

		// main window.
		SDL_Window* mainwin = nullptr;
		float scaling;

		// last directory.
		std::string lastpath = "."; // defaults to current directory.

		bool	romChange = false;
		std::string changeRomTo = "";

		// nsf
		int		zeroIndexedTrackNo;
		bool	repeatTrackAfterEnd15s = false;
		
	};

	void	render_main (nes_emulator *denver,  GLuint tex, denvergui_state *state);
};

