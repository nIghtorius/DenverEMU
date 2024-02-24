/*

	debug_renderer.h:
		class(es) for visualising the name tables and pattern tables.

*/

#pragma once

#include "ppu.h"				// we should be able to access the video ram/rom/etc.
#include <cstdint>

class ppu_debug_vram {
private:
	ppu			*target;
	uint32_t	*patterntable;
	uint32_t	*nametable;
public:
	ppu_debug_vram();
	~ppu_debug_vram();
	bool		show_scroll_registers = false;
	bool		show_ppu_updates_from_cpu = false;
	void		set_target_ppu(ppu *target_ppu = nullptr);
	void		render_tilemap(uint8_t palette);
	void		render_nametable(); // full 2x2 render! 512x480pixels.
	uint32_t*	get_patterntable_render();
	uint32_t*	get_nametable_render();
};
