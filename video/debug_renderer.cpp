/*

	debug_renderer.cpp implementation

*/

#include "debug_renderer.h"
#include "nesvideo.h"		// we need the colors.
#include <cstdlib>

ppu_debug_vram::ppu_debug_vram() {
	// alloc 256x128 32b pixels.
	patterntable = (uint32_t*)malloc(256 * 128 * 4);
	// alloc 512x480 32b pixels.
	nametable = (uint32_t*)malloc(512 * 480 * 4);
}

ppu_debug_vram::~ppu_debug_vram() {
	// free up resources.
	free(patterntable);
	free(nametable);
}

void	ppu_debug_vram::set_target_ppu(ppu *target_ppu) {
	target = target_ppu;
}

void	ppu_debug_vram::render_tilemap(uint8_t palette) {
	// check if a ppu is set.
	if (!target) return;

	// renders the tilemap when the given palette ID.
	uint8_t colors[4];
	
	// read the colors.
	for (int i = 0; i < 4; i++) 
		colors[i] = target->vpal.read(palette * 4 + i, palette * 4 + i, true) & 0x3F;

	// precompute the colors.
	uint32_t ccolors[4];
	for (int i = 0; i < 4; i++) {
		uint32_t idx = colors[i] * 3;
		ccolors[i] = 0xFF000000 | ((ntscpalette[idx + 2]) << 16) |
			((ntscpalette[idx + 1]) << 8) |
			((ntscpalette[idx]));
	}

	// when reading pattern tables. first 8 bytes is bit 0, next 8 bytes is bit 1 (4 color)
	uint8_t tilemap[16];

	// left / right patterntables.
	for (int pat = 0; pat < 2; pat++) {
		int pattern_start = pat * 0x1000;
		for (int tile = 0; tile < 256; tile++) {
			for (int i = 0; i < 16; i++) {
				tilemap[i] = target->vbus.readmemory(pattern_start + (tile << 4) + i, true);
			}
			// tile is read. render to the visualiser buffer.
			int tile_x = tile % 16;		// get x position
			int tile_y = tile / 16;		// get y position
			int start = (tile_x << 3) + ((tile_y << 3) << 8);	// x*8+y*256
			if (pat == 1) start += 128;	// increment x position

			for (int i = 0; i < 8; i++) {
				// render the pixels.
				for (int j = 0; j < 8; j++) {
					uint8_t pixel = tilemap[i] & 1 | ((tilemap[i + 8] & 1) << 1);
					tilemap[i] >>= 1;
					tilemap[i + 8] >>= 1;
					patterntable[start + 7 - j] = ccolors[pixel];
				}
				start += 256;	// increase Y position (stride)
			}
		}
	}
}

void	ppu_debug_vram::render_nametable() {
	// renders the entire nametable.
	// name table is 64x60. We are just going to render that.
	// the ppu object will automaticly handle all mapper stuff that is connected to it.
	if (!target) return;	// we need a connected ppu.

	uint32_t	ccolors[64];

	// precompute color table.
	for (int i = 0; i < 64; i++) {
		uint32_t idx = i * 3;
		ccolors[i] = 0xFF000000 | ((ntscpalette[idx + 2]) << 16) |
			((ntscpalette[idx + 1]) << 8) |
			((ntscpalette[idx]));
	}
	
	for (int table = 0; table < 4; table++) {		
		int		addr_start = 0x2000;
		int		start_x, start_y = 0;
		switch (table) {
		case 0:
			addr_start = 0x2000;
			start_x = 0;
			start_y = 0;
			break;
		case 1:
			addr_start = 0x2400;
			start_x = 32;
			start_y = 0;
			break;
		case 2:
			addr_start = 0x2800;
			start_x = 0;
			start_y = 30;
			break;
		case 3:
			addr_start = 0x2C00;
			start_x = 32;
			start_y = 30;
			break;
		}
		for (int y = 0; y < 30; y++) {
			for (int x = 0; x < 32; x++) {
				int nt_addr = addr_start + (y * 32) + x;
				int at_addr = addr_start + 0x03C0 + (y/4)*8 + (x / 4);
				uint8_t	tile = target->vbus.readmemory(nt_addr, true);
				uint8_t attr = target->vbus.readmemory(at_addr, true);

				// we now have a tile and a attribute. read pattern.
			
				// first where is the bg pattern table?
				int	pattern_start = target->ppuctrl.bg_0x1000 ? 0x1000 : 0;

				// read tile.
				uint8_t tilemap[16];
				for (int i = 0; i < 16; i++)
					tilemap[i] = target->vbus.readmemory(pattern_start + (tile << 4) + i, true);

				// a single attribute file contains 4 attributes.
				// one attribute is for 2 tiles. 32 tiles = 16 attributes.
				// 4 attribute per bytes thus 4 bytes. 16 / 4 = 4. (<< 2)
				int at_bit_part = (x >> 1) % 2;	// 0, 1
				at_bit_part |= ((y >> 1) % 2) << 1; // 0, 1, 2, 3

				uint8_t	color = (attr >> (at_bit_part << 1)) & 0x03;		// get color group.

				// render tile.
				int		sstart = ((start_x + x) << 3) + (((start_y + y) << 3) << 9);

				for (int i = 0; i < 8; i++) {
					// render the pixels.
					for (int j = 0; j < 8; j++) {
						uint8_t pixel = tilemap[i] & 1 | ((tilemap[i + 8] & 1) << 1);
						tilemap[i] >>= 1;
						tilemap[i + 8] >>= 1;
						pixel = target->vpal.read(color * 4 + pixel, color * 4 + pixel, true) & 0x3F;
						nametable[sstart + 7 - j] = ccolors[pixel];
					}
					sstart += 512;	// increase Y position (stride)
				}

			}
		}
	}

	// show scroll regs?
	if (show_scroll_registers) {
		uint16_t	loopy_v = target->ppu_internal.v_register & ~0x7BE0 | (target->ppu_internal.t_register & 0x7BE0);
		loopy_v = loopy_v & ~0x041F | (target->ppu_internal.t_register & 0x041F);
		// we can compute the X, Y scrolls.
		int course_x = loopy_v & 0x1F;
		int course_y = (loopy_v & 0x03E0) >> 5;
		int nt = (loopy_v & 0x0C00) >> 10;
		int fine_y = (loopy_v & 0x7000) >> 12;
		int fine_x = target->ppu_internal.x_shift;

		int x = (course_x << 3) + fine_x;
		int y = (course_y << 3) + fine_y;

		switch (nt) {
		case 1:
			x += 256;
			break;
		case 2:
			y += 240;
			break;
		case 3:
			x += 256;
			y += 240;
		}

		// draw the lines.
		// x scroll.
		int start = x;
		for (int i = 0; i < 480; i++) {
			nametable[start] = 0xFFFF0000;
			start += 512;
		}

		// y scroll.
		start = y * 512;
		for (int i = 0; i < 512; i++) {
			nametable[start++] = 0xFF00FF00;
		}
	}
}

uint32_t*	ppu_debug_vram::get_patterntable_render() {
	return patterntable;
}

uint32_t*	ppu_debug_vram::get_nametable_render() {
	return nametable;
}