#pragma once

#include "../emulator/nes.h"

namespace denvergui {
	struct denvergui_state {
		bool show_apu_debugger;
		bool apu_tab_pulse1;
		bool apu_tab_pulse2;
		bool apu_tab_tri;
		bool apu_tab_noi;
		bool apu_tab_dmc;
	};

	void	render_main (nes_emulator *denver,  GLuint tex, denvergui_state *state);
};

