#include "ppu.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#pragma warning(disable : 4996)

ppu::ppu() : bus_device () {
	strncpy(get_device_descriptor(), "Denver PPU Unit", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x2000;
	deviceend = 0x2FFF;
	devicemask = 0x2FFF;
	tick_rate = 0x1;
	// make bus
	// Palette RAM
	vbus.registerdevice(&vpal);
	// vram
	vbus.registerdevice(&vram);
	// oam (internal, unbussed)
	// framebuffer.
	framebuffer = (word *)malloc(256 * 240 * 2);
	scanline = -1;
	beam = 0;
	cycle = 0;
	// initialize debug lists.
	events.clear();
}

ppu::~ppu() {
	vbus.removedevice_select_base(vpal.devicestart);
	vbus.removedevice_select_base(vram.devicestart);
}

void ppu::write_state_dump (const char *filename) {
		std::ofstream ppu_dump;
		ppu_dump.open(filename, std::ios::binary | std::ios::out);
		ppu_dump.write((char *)&ppu_internal, sizeof(ppu_render_state));
		ppu_dump.close();
}


byte	ppu::read(int addr, int addr_from_base, bool onlyread) {
	// registers
	// PPU STATUS register
	if (addr_from_base == PPU_PPUSTATUS_PORT) {
		byte	status = latch & 0x1F;
		if (ppustatus.sprite_0_hit) status |= PPU_SPR_0HIT;
		if (ppustatus.sprite_overflow) status |= PPU_SPR_OVERFLOW;
		if (ppustatus.vblank) status |= PPU_VBLANK;	
		// clear vblank status after reading PPUSTATUS_PORT (* see https://wiki.nesdev.com/w/index.php/PPU_registers @ read $2002)
		ppustatus.vblank = false;
		ppu_internal.address_write_latch = false;	// reset write latch for address and scroll (* see https://wiki.nesdev.com/w/index.php/PPU_registers @ read $2002)
		return status;
	}
	// OAMDATA register
	if (addr_from_base == PPU_OAMDATA_PORT) {
		if (ppu_internal.oam_clearing) return 0xFF; // when clearing sec oam, reads to 2004 will return 0xFF
		char *buf = (char *)&oam;
		return buf[oamaddr]; // no increment as per write (* see https://wiki.nesdev.com/w/index.php/PPU_registers @ read $2004)
	}
	// READ register.
	if (addr_from_base == PPU_DATA_PORT) {
		byte data = 0;
		if (ppu_internal.v_register < 0x3F00) {
			data = prt2007buffer;
			prt2007buffer = vbus.readmemory(ppu_internal.v_register & 0x3FFF);
		}
		else {
			// special case.
			data = vpal.read(ppu_internal.v_register & 0x3F1F, (ppu_internal.v_register & 0x3F1F) - 0x3F00, false);
			prt2007buffer = vbus.readmemory(ppu_internal.v_register & 0x2FFF);
		}
		if (ppuctrl.increment_32_bytes) {
			ppu_internal.v_register += 32;
		}
		else {
			ppu_internal.v_register++;
		}
		return data;
	}
	return 0x00;
}

void	ppu::write(int addr, int addr_from_base, byte data) {
	// update latch
	latch = data;

	// this is an event. therefore log it. unless it is a data write.
	if (addr_from_base != PPU_DATA_PORT) {
		ppu_cpu_event event;
		event.ppuaddr = addr;
		event.ppucycle = ppu_cycles_per_frame;
		event.cpucycle = 0;
		event.scanline = scanline;
		event.beam = beam;
		event.data = data;
		// add this to the events list.
		events.push_back(event);
	}

	// registers.
	if (addr_from_base == PPU_PPUCTRL_PORT) {
		ppu_internal.t_register &= ~0xC00;
		ppu_internal.t_register |= (data & PPU_BASENAMETABLE) << 10;
		ppuctrl.bg_0x1000 = (data & PPU_BG_TABLE_0X1000) > 0;
		ppuctrl.sprites_0x1000 = (data & PPU_SPRITE_TABLE_0X1000) > 0;
		ppuctrl.do_nmi = (data & PPU_DO_NMI) > 0;
		ppuctrl.increment_32_bytes = (data & PPU_VRAM_INCREMENT_32BYTES) > 0;
		ppuctrl.master_mode = (data & PPU_MASTER_MODE) > 0;
		ppuctrl.sprites_8x16 = (data & PPU_SPRITE_8X16) > 0;
	}
	if (addr_from_base == PPU_PPUMASK_PORT) {
		ppumask.bg8lt = (data & PPU_BG_L8P) > 0;
		ppumask.emp_blu = (data & PPU_EMP_BLUE) > 0;
		ppumask.emp_grn = (data & PPU_EMP_GREEN) > 0;
		ppumask.emp_red = (data & PPU_EMP_RED) > 0;
		ppumask.grayscale = (data & PPU_GREYSCALE) > 0;
		bool	prev_state_show = ppumask.showbg || ppumask.showspr;
		ppumask.showbg = (data & PPU_SHOW_BG) > 0;
		ppumask.showspr = (data & PPU_SHOW_SPR) > 0;
		if ((ppumask.showbg || ppumask.showspr) && !prev_state_show) {
			// reset rendering pipeline evaluators.
			ppu_internal.n = 0;
			ppu_internal.sn = 0;
			ppu_internal.m = 0;
		}
		ppumask.spr8lt = (data & PPU_SPR_L8P) > 0;
	}
	// OAMADDR register (0x03)
	if (addr_from_base == PPU_OAMADDR_PORT) {
		oamaddr = data;
	}
	// OAMDATA register
	if (addr_from_base == PPU_OAMDATA_PORT) {
		char *buf = (char*)&oam;
		buf[oamaddr++] = data;
	}
	if (addr_from_base == PPU_SCROLL_PORT) {
		if (!ppu_internal.address_write_latch) {
			ppu_internal.t_register &= ~0x1F;
			ppu_internal.t_register |= (data & 0xF8) >> 3;
			ppu_internal.x_shift = data & 0x07;
		}
		else {
			ppu_internal.t_register &= ~0x73E0;
			ppu_internal.t_register |= (data & 0x07) << 12;
			ppu_internal.t_register |= (data & 0xF8) << 2;
		}
		ppu_internal.address_write_latch = !ppu_internal.address_write_latch;
	}
	if (addr_from_base == PPU_ADDRESS_PORT) {
		if (!ppu_internal.address_write_latch) {
			ppu_internal.t_register &= ~0x7F00;
			ppu_internal.t_register |= (data & 0x3F) << 8;
		}
		else {
			ppu_internal.t_register &= ~0xFF;
			ppu_internal.t_register |= data;
			ppu_internal.v_register = ppu_internal.t_register;
		}
		ppu_internal.address_write_latch = !ppu_internal.address_write_latch;
	}
	if (addr_from_base == PPU_DATA_PORT) {
		vbus.writememory(ppu_internal.v_register & 0x3FFF, data);
		ppu_internal.v_register += ppuctrl.increment_32_bytes ? 32 : 1;
	}
}

int		ppu::rundevice(int ticks) {
	// run the PPU..
	for (int i = 0; i < ticks; i++) {
		if ((ppumask.showbg || ppumask.showspr) && !((scanline>=240) && (scanline<=260))) {
			// loading cycles.
			if (((cycle >= 1) && (cycle <= 256)) | ((cycle >= 321) && (cycle <= 340))) {
				if (((cycle - 1) % 8) == 1) {
					// load name table shift register.
					ppu_internal.shiftreg_nametable = vbus.readmemory(0x2000 | (ppu_internal.v_register & 0x0FFF));
				}
				else if (((cycle - 1) % 8) == 3) {					
					// load attribute table shift register.
					word attr_address = ppu_internal.v_register & ~0x2000;
					attr_address = 0x03C0 | (attr_address & 0x0C00) | ((attr_address >> 4) & 0x0038) | ((attr_address >> 2) & 0x0007);
					// better fetch the palette entry @ latch mode, finding it later is incredibly difficult.
					byte ab = vbus.readmemory(attr_address | 0x2000);
					ppu_internal.shiftreg_attribute_latch = ab;
				}
				else if (((cycle - 1) % 8) == 5) {
					// load pattern table tile low
					ppu_internal.y_shift = (ppu_internal.v_register & 0x7000) >> 12;
					word pattern_address = (ppuctrl.bg_0x1000 << 12) | ((word)ppu_internal.shiftreg_nametable << 4) | ppu_internal.y_shift;
					// ppu_internal.shiftregs_pattern_par[0] = vbus->readmemory(pattern_address);<-- hack
					ppu_internal.shiftregs_pattern_latch = (vbus.readmemory(pattern_address)); // read to lower part.
				}
				else if (((cycle - 1) % 8) == 7) {
					// load pattern table tile high
					word pattern_address = (ppuctrl.bg_0x1000 << 12) | ((word)ppu_internal.shiftreg_nametable << 4) | 8 | ppu_internal.y_shift;
					//ppu_internal.shiftregs_pattern_par[1] = vbus->readmemory(pattern_address);
					if (cycle >= 321) {
						ppu_internal.shiftregs_pattern[0] <<= 8; // clean upper part.
						ppu_internal.shiftregs_pattern[1] <<= 8; // clean upper part.
						ppu_internal.shiftreg_attribute[0] <<= 8; // clean upper part.
						ppu_internal.shiftreg_attribute[1] <<= 8; // clean upper part.
					}
					ppu_internal.shiftregs_pattern[1] |= (vbus.readmemory(pattern_address)); // read to lower part.
					ppu_internal.shiftregs_pattern[0] |= ppu_internal.shiftregs_pattern_latch;

					byte color = ((ppu_internal.shiftreg_attribute_latch) >> (((((ppu_internal.v_register & 0x1F) >> 1) % 2) << 1) | ((((ppu_internal.v_register & 0x3E0) >> 6) % 2) << 2)) & 0x03);
					byte cb1 = color & 0x01;
					byte cb2 = (color >> 1) & 0x01;					
					ppu_internal.shiftreg_attribute[0] |= cb1 | (cb1 << 1) | (cb1 << 2) | (cb1 << 3) | (cb1 << 4) | (cb1 << 5) | (cb1 << 6) | (cb1 << 7);
					ppu_internal.shiftreg_attribute[1] |= cb2 | (cb2 << 1) | (cb2 << 2) | (cb2 << 3) | (cb2 << 4) | (cb2 << 5) | (cb2 << 6) | (cb2 << 7);

					// last fetch also update Coarse X (v register) (loopy_v verti)
					if ((ppu_internal.v_register & 0x001F) == 31) {
						ppu_internal.v_register &= ~0x001F;
						ppu_internal.v_register ^= 0x0400;
					}
					else {
						ppu_internal.v_register++;
					}
				}
			}
			// secondary OAM reset (cycli 1--64)
			if ((cycle >= 1) && (cycle <= 64)) {
				ppu_internal.oam_clearing = true;
				byte secaddr = (cycle - 1) % 8;
				memset(&ppu_internal.secoam[(cycle - 1) % 8], 0xFF, 4);
				ppu_internal.n = 0;
				ppu_internal.m = 0;
				ppu_internal.sn = 0;
				ppu_internal.oam_evald = false;
			}
			// sprite eval (cycli 65--256)
			if ((cycle >= 65) && (cycle <= 256)) {				
				ppu_internal.oam_clearing = false;
				bool odd = (cycle % 2) > 0;
				if (odd) {
					// odd cycle (read from primary OAM)
					ppu_internal.secoamb = (byte *)&oam[ppu_internal.n];
					ppu_internal.buffer_oam_read = ppu_internal.secoamb[ppu_internal.m];
				} else {
					// even cycle (write to secondary OAM)
					if (ppu_internal.m == 0) {
						int scancomp = (scanline == 261) ? -1 : scanline;
						if ((ppu_internal.sn < 8) && !ppu_internal.oam_evald) ppu_internal.secoam[ppu_internal.sn].y = ppu_internal.buffer_oam_read;
						ppu_internal.oam_copy =
							((scancomp >= ppu_internal.buffer_oam_read) &&
							(scancomp < (int)ppu_internal.buffer_oam_read + (ppuctrl.sprites_8x16 ? 16 : 8))) && (scancomp < 239);
						if (ppu_internal.oam_copy && (ppu_internal.sn >= 8)) {
							// sprite overflow.
							ppustatus.sprite_overflow = true;
						}
					}
					// copy cycles.
					if (ppu_internal.oam_copy) {
						if ((ppu_internal.sn < 8) && !ppu_internal.oam_evald) {
							switch (ppu_internal.m) {
							case 0x01:	
								ppu_internal.secoam[ppu_internal.sn].tile = ppu_internal.buffer_oam_read;
								break;
							case 0x02:	
								ppu_internal.secoam[ppu_internal.sn].attr = ppu_internal.buffer_oam_read;
								// infuse spr 0 status into the attr (internal use only)
								if (ppu_internal.n == 0) ppu_internal.secoam[ppu_internal.sn].attr |= OAM_SPR_INTERNAL_ONLY_SPR0F;
								break;
							case 0x03:	
								ppu_internal.secoam[ppu_internal.sn].x = ppu_internal.buffer_oam_read;
								break;
							}
						}
						ppu_internal.m++;
						if (ppu_internal.m == 4) {
							// m overflows, reset to 0 and increment n.
							ppu_internal.m = 0;
							ppu_internal.n++;
							if (ppu_internal.sn < 8) ppu_internal.sn++;
							ppu_internal.oam_copy = false;	// disable copy system.
						}
					}
					else {
						// wrap n
						// increment n when no copy is required.
						ppu_internal.n++;
					}
					// spr overflow bug?
					//if (ppustatus.sprite_overflow && (ppu_internal.sn < 8)) ppu_internal.n++;
					// wrap n
					if (ppu_internal.n == 64) {
						ppu_internal.oam_evald = true;
						ppu_internal.n = 0;
					}
				}
			}
			// sprite loading.
			if ((cycle >= 257) && (cycle <= 320) && (scanline != 261)) {
				if (cycle == 257) ppu_internal.n = 0;
				byte cs = (cycle - 1) % 8;
				if (cs == 1) {
					// do garbage nametable read.
					//ppu_internal.shiftreg_nametable = vbus.readmemory(0x2000 | (ppu_internal.v_register & 0x0FFF));
					ppu_internal.spr_pix = 0; // reset sprite render buffer. it's a denver thing.
				}
				else if (cs == 3) {
					// do garbage nametable read.
					//ppu_internal.shiftreg_nametable = vbus.readmemory(0x2000 | (ppu_internal.v_register & 0x0FFF));
					ppu_internal.shiftreg_spr_latch[ppu_internal.n] = ppu_internal.secoam[ppu_internal.n].attr;
				}
				else if (cs == 5) {
					// load pattern table tile low
					byte ltile = ppu_internal.secoam[ppu_internal.n].tile;
					int	 ix = scanline - (ppu_internal.secoam[ppu_internal.n].y);
					word pattern_address;
					if (ppuctrl.sprites_8x16) {
						pattern_address = (ltile & 1) == 1 ? 0x1000 : 0x0000;
						ltile &= 0xFE;
					}
					else {
						pattern_address = (ppuctrl.sprites_0x1000 << 12);
					}
					// check bit flip horiz.
					if ((ppu_internal.shiftreg_spr_latch[ppu_internal.n] & OAM_SPR_ATTR_FLIP_VER) == OAM_SPR_ATTR_FLIP_VER) {
						// inverse ix.
						ix = ppuctrl.sprites_8x16 ? 15 - ix : 7 - ix;
					}
					if (ix > 7) pattern_address += 8;
					pattern_address += (ltile << 4) + ix;
					if (pattern_address >= 0x1FF8) pattern_address = 0x1FF8; //clamp it.
					ppu_internal.shiftreg_spr_pattern_lo[ppu_internal.n] = vbus.readmemory(pattern_address);
					ppu_internal.shiftreg_spr_counter[ppu_internal.n] = ppu_internal.secoam[ppu_internal.n].x;
				}
				else if (cs == 7) {
					// load pattern table tile high
					byte ltile = ppu_internal.secoam[ppu_internal.n].tile;
					int	 ix = scanline - (ppu_internal.secoam[ppu_internal.n].y);
					word pattern_address;
					if (ppuctrl.sprites_8x16) {
						pattern_address = (ltile & 1) == 1 ? 0x1000 : 0x0000;
						ltile &= 0xFE;
					}
					else {
						pattern_address = (ppuctrl.sprites_0x1000 << 12);
					}
					// check bit flip horiz.
					if ((ppu_internal.shiftreg_spr_latch[ppu_internal.n] & OAM_SPR_ATTR_FLIP_VER) == OAM_SPR_ATTR_FLIP_VER) {
						// inverse ix.
						ix = ppuctrl.sprites_8x16 ? 15 - ix : 7 - ix;
					}					
					if (ix > 7) pattern_address += 8;
					pattern_address += (ltile << 4) + ix;
					if (pattern_address >= 0x1FF8) pattern_address = 0x1FF8; //clamp it.
					ppu_internal.shiftreg_spr_pattern_hi[ppu_internal.n] = vbus.readmemory(pattern_address + 8);
					ppu_internal.shiftreg_spr_counter[ppu_internal.n] = ppu_internal.secoam[ppu_internal.n].x;
					ppu_internal.n++;	// eval to next sprite in seconday oam. this will never go over 7, because eval will stop earlier.
				}
			}
			// addressing cycle.
			if (cycle == 257) {
				ppu_internal.v_register = (ppu_internal.v_register & ~0x041F) | (ppu_internal.t_register & 0x041F);
			}
			// loopy_v cycle (hori) @ 257
			//std::cout << std::dec << cycle << ", ";
			if (cycle == 256) {
				if ((ppu_internal.v_register & 0x7000) != 0x7000) {
					ppu_internal.v_register += 0x1000;
				}
				else {
					ppu_internal.v_register &= ~0x7000;
					int y = ((ppu_internal.v_register & 0x03E0) >> 5);
					if (y == 29) {
						y = 0;
						ppu_internal.v_register ^= 0x0800;
					}
					else if (y == 31) {
						y = 0;
					}
					else {
						y++;
					}
					ppu_internal.v_register = (ppu_internal.v_register & ~0x03E0) | (y << 5);
				}
			}
		}
		// BG/SPR rendering cycles.
		if ((scanline >= 0) && (scanline <= 239)) {
			if ((cycle >= 0) && (cycle <= 259)) {
				if (ppumask.showspr) {
					// decrement counters.
					for (int i = 7; i >= 0; i--) {
						// check if count is zero then render the pattern buffers.
						if (ppu_internal.shiftreg_spr_counter[i] == 0) {
							// we can get the pixel two ways depending on OAM_SPR_ATTR_FLIP_VER
							byte pix = 0;
							if ((ppu_internal.shiftreg_spr_latch[i] & OAM_SPR_ATTR_FLIP_HOR) == OAM_SPR_ATTR_FLIP_HOR) {
								// flipped.
								pix = ppu_internal.shiftreg_spr_pattern_lo[i] & 0x01;
								pix |= (ppu_internal.shiftreg_spr_pattern_hi[i] & 0x01) << 1;
								ppu_internal.shiftreg_spr_pattern_lo[i] >>= 1;
								ppu_internal.shiftreg_spr_pattern_hi[i] >>= 1;
							}
							else {
								// normal.
								pix = (ppu_internal.shiftreg_spr_pattern_lo[i] & 0x80) >> 7;
								pix |= (ppu_internal.shiftreg_spr_pattern_hi[i] & 0x80) >> 6;
								ppu_internal.shiftreg_spr_pattern_lo[i] <<= 1;
								ppu_internal.shiftreg_spr_pattern_hi[i] <<= 1;
							}

							// get pixel color.
							byte spr_palette = (ppu_internal.shiftreg_spr_latch[i] & OAM_SPR_ATTR_PALETTE);
							if (pix > 0) {
								ppu_internal.spr_pix = pix;
								if ((ppu_internal.shiftreg_spr_latch[i] & OAM_SPR_PRIORITY) > 0) ppu_internal.spr_pix |= 0x80;
								if ((ppu_internal.shiftreg_spr_latch[i] & OAM_SPR_INTERNAL_ONLY_SPR0F) > 0) ppu_internal.spr_pix |= OAM_SPR_INTERNAL_ONLY_SPR0F;
								ppu_internal.spr_pix_pal = vpal.read(0, 0x10 | spr_palette << 2 | pix);
							}
						}
						// decrement counter register.
						if (ppu_internal.shiftreg_spr_counter[i] < 255)
							ppu_internal.shiftreg_spr_counter[i] = ppu_internal.shiftreg_spr_counter[i] > 0 ? ppu_internal.shiftreg_spr_counter[i] - 1 : 0;
					}
				}
				if (ppumask.showbg) {				
					// get the pixel.
					word pix_mux = 0x8000 >> ppu_internal.x_shift;
					byte palentry = (((ppu_internal.shiftregs_pattern[1] & pix_mux) > 0) << 1) |
						((ppu_internal.shiftregs_pattern[0] & pix_mux) > 0);

					byte color = (((ppu_internal.shiftreg_attribute[1] & pix_mux) > 0) << 1) |
						((ppu_internal.shiftreg_attribute[0] & pix_mux) > 0);
					
					color <<= 2;
					

					// convert color from the palette entry!
					//color = vbus.readmemory ((color | 0x3F00) | palentry);
					// no need for full bus emulation on color data.
					color = vpal.read(0, color | palentry);	// addr = 0x000 (vpal ignores that anyway)

					// shift the registers.
					ppu_internal.shiftregs_pattern[0] <<= 1;
					ppu_internal.shiftregs_pattern[1] <<= 1;
					ppu_internal.shiftreg_attribute[0] <<= 1;
					ppu_internal.shiftreg_attribute[1] <<= 1;

					// check disabled 8 left pixels.
					if ((!ppumask.bg8lt) && (beam < 8)) {
						color = vpal.read(0, 0);
						palentry = 0;
					}
					if ((!ppumask.spr8lt) && (beam < 8)) ppu_internal.spr_pix = 0;

					// draw to framebuffer?
					if (beam<256) {
						framebuffer[scanline << 8 | beam] = color; // (palentry << 4) | (palentry << 4) << 8;
						framebuffer[scanline << 8 | beam] |= (ppumask.emp_blu ? 0x0100 : 0) |
							(ppumask.emp_grn ? 0x0200 : 0) |
							(ppumask.emp_red ? 0x0400 : 0);

						// spr_pix>0 just render for now.
						bool renderpixel = ((palentry == 0) && (ppu_internal.spr_pix > 0) ||
							(palentry > 0) && ((ppu_internal.spr_pix & 0x80) == 0) && ((ppu_internal.spr_pix & 0x03) > 0));

						if (renderpixel) {
							framebuffer[scanline << 8 | beam] = ppu_internal.spr_pix_pal;
						}						
						// check for spr 0
						if (((ppu_internal.spr_pix & 0x3) > 0) && (palentry != 0)) {
							if ((ppu_internal.spr_pix & OAM_SPR_INTERNAL_ONLY_SPR0F) && (beam<255)) ppustatus.sprite_0_hit = true;
						}
						ppu_internal.spr_pix = 0;
					}
				}
				else framebuffer[scanline << 8 | beam] = vpal.read(0, 0);
				beam++;
			}
		}
		// flags and nmi.
		if ((scanline == 241) && (cycle == 1)) {
			// set vblank flag and raise NMI if enabled.
			ppustatus.vblank = true;
			if (ppuctrl.do_nmi) nmi_enable = true;	// nmi enabled gets automaticly pulled up by bus device.
		}
		if (scanline == 261) {
			if (cycle <= 1) {
				// reset flags.
				ppustatus.vblank = false;
				ppustatus.sprite_0_hit = false;
				ppustatus.sprite_overflow = false;
			}
			if ((cycle >= 280) && (cycle <= 304)) {
				// reload v register from parts of the t register.
				if (ppumask.showbg || ppumask.showspr) {
					ppu_internal.v_register = (ppu_internal.v_register & ~0x7BE0) | (ppu_internal.t_register & 0x7BE0);
				}
			}
		}

		// Scanline..
		if (cycle == 339) {
			scanline++;
			beam = 0;
			// framebuffer done.
			if (scanline == 240) {
				frameready = true; // frame is ready also check the callback.
				if (callback) callback();
			}
			if (scanline == 262) {
				events.clear();	// clear captured events.
				scanline = 0;
				if (ppu_internal.odd_even_frame) cycle = 340;
				ppu_internal.odd_even_frame = !ppu_internal.odd_even_frame;
			}
		}

		// snap cycle?
		if (ppu_cycles_per_frame == capture_cycle) {
			snap_state_for_debugger();
		}

		cycle++; 
		ppu_cycles_per_frame++;

		if (cycle == 341) {			
			cycle = 0;
			if (scanline == 0) {
				ppu_cycles_per_frame = 0;
			}
		}
	}
	return ticks;	// assume ticks in = ticks out.
}

void	ppu::snap_state_for_debugger() {
	// snaps all render values to a buffer for debugging purposes.
	// copy ctrl registers.
	memcpy(&dbg_ppuctrl, &ppuctrl, sizeof(ppu_ctrl_register));
	memcpy(&dbg_ppumask, &ppumask, sizeof(ppu_mask_register));
	memcpy(&dbg_ppustatus, &ppustatus, sizeof(ppu_status_register));
	memcpy(&dbg_ppuint, &ppu_internal, sizeof(ppu_render_state));
	// copy emulation registers (those are not official)
	dbg_latch = latch;
	dbg_p2007buf = prt2007buffer;
	dbg_ppuaddr = ppuaddr;
	dbg_sl = scanline;
	dbg_beam = beam;
	dbg_cycle = cycle;
	// render a debugger.
	if (dbg_callback) dbg_callback();
}

void	ppu::set_char_rom(bus_device *vdata) {
	// first remove any linked rom/rams!
	// it is destructive.
	vbus.removedevice_select_base(0x0000);
	// reregister	
	vbus.registerdevice(vdata);
}

void	ppu::configure_vertical_mirror() {
	vram.resetpins_to_default();
}

void	ppu::configure_horizontal_mirror() {
	vram.resetpins_to_default();
	vram.swappins(10, 11);
	vram.groundpin(10);
}

void	ppu::dma(byte *data, bool is_output, bool started) {
	if (is_output) {
		char * buf = (char *)oam;
		buf[oamaddr++] = *data;
	}
}

void*	ppu::getFrameBuffer() {
	return framebuffer;
}

bool	ppu::isFrameReady() {
	bool retval = frameready;
	frameready = false;
	return retval;
}

void	ppu::reset() {
	cycle = 0;
	memset (&ppu_internal, 0, sizeof (ppu_render_state));
	nmi_enable = false;
	irq_enable = false;
	ppuctrl.do_nmi = false;
}

// PPU RAM
ppuram::ppuram() : bus_device() {
	strncpy(get_device_descriptor(), "PPU mainram 2k", MAX_DESCRIPTOR_LENGTH);
	ram = (byte *)malloc(0x800);
	devicestart = 0x2000;
	deviceend = 0x37FF;
	devicemask = 0x27FF;	 // per default ppu has 2k of RAM mirrored to 4k of address-space.
}

ppuram::~ppuram() {
	free(ram);
}

void	ppuram::write(int addr, int addr_from_base, byte data) {
	ram[addr_from_base] = data;
}

byte	ppuram::read(int addr, int addr_from_base, bool onlyread) {
	return ram[addr_from_base];
}

// PPU PAL RAM
ppu_pal_ram::ppu_pal_ram() : bus_device() {
	strncpy(get_device_descriptor(), "PPU palette RAM 32 bytes", MAX_DESCRIPTOR_LENGTH);
	ram = (byte *)malloc(0x20);
	devicestart = 0x3F00;
	deviceend = 0x3FFF;
	devicemask = 0x3F1F;
}

ppu_pal_ram::~ppu_pal_ram() {
	free(ram);
}

int ppu_pal_ram::pal_addr_compute(int addr) {
	if (addr == 0x10) return 0x00;
	if (addr == 0x14) return 0x04;
	if (addr == 0x18) return 0x08;
	if (addr == 0x1C) return 0x0C;
	return addr;
}

void ppu_pal_ram::write(int addr, int addr_from_base, byte data) {
	ram[pal_addr_compute (addr_from_base)] = data;
}

byte ppu_pal_ram::read(int addr, int addr_from_base, bool onlyread) {
	if (addr_from_base == 0x04) return ram[0x00];
	if (addr_from_base == 0x08) return ram[0x00];
	if (addr_from_base == 0x0C) return ram[0x00];
	return ram[pal_addr_compute(addr_from_base)];	
}

