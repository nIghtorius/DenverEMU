#pragma once

#include "../emulator/nes.h"
#include "../cpu/tools/2a03_disasm.h"
#include "dengui_main.h"

namespace denvergui {
	void	render_cpuviewer(nes_emulator *denver, denvergui_state *state);
	void	render_apuviewer(nes_emulator *denver, denvergui_state *state);
	void	render_ppuviewer(nes_emulator *denver, denvergui_state *state);
}