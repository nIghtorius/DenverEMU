#pragma once

#include "../emulator/nes.h"
#include "../cpu/tools/2a03_disasm.h"
#include "dengui_main.h"
#include "../debug/device_debugger.h"

namespace denvergui {
	void	render_cpuviewer(nes_emulator *denver, denvergui_state *state);
	void	render_apuviewer(nes_emulator *denver, denvergui_state *state);
	void	render_ppuviewer(nes_emulator *denver, denvergui_state *state);
	void	render_debugviewer(device_debugger* device, bool* showhide, char* device_name);
	void	internal_render_dbg_node(dbg_dt* node);
}