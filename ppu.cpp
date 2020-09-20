#include "stdafx.h"
#include "ppu.h"
#include <malloc.h>
#include <iostream>
#include <fstream>


#include <thread>
#include <chrono>

ppu::ppu() : bus_device () {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "Denver PPU Unit");
	this->devicestart = 0x2000;
	this->deviceend = 0x3FFF;
	this->devicemask = 0x2007;
	this->tick_rate = 0x1;
	// make bus
	vbus = new bus();
	// Palette RAM
	vpal = new ppu_pal_ram();
	vbus->registerdevice(vpal);
	// vram
	vram = new ppuram();
	vbus->registerdevice(vram);
	// oam (internal, unbussed)
	oam = (oamentry *)malloc(256);	 // reserve 256 bytes for OAM memory.	
	// framebuffer.
	framebuffer = (word *)malloc(256 * 240 * 2);
	// write stripes.
	memset(framebuffer, 0xF0, 256 * 240 * 2);
	scanline = -1;
	beam = 0;
	cycle = 0;
}

byte	ppu::read(int addr, int addr_from_base) {
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
		char *buf = (char *)oam;
		return buf[oamaddr]; // no increment as per write (* see https://wiki.nesdev.com/w/index.php/PPU_registers @ read $2004)
	}
	// READ register.
	if (addr_from_base == PPU_DATA_PORT) {
		byte data = prt2007buffer;
		prt2007buffer = vbus->readmemory(ppu_internal.v_register);
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
		ppumask.showbg = (data & PPU_SHOW_BG) > 0;
		ppumask.showspr = (data & PPU_SHOW_SPR) > 0;
		ppumask.spr8lt = (data & PPU_SPR_L8P) > 0;
	}
	// OAMADDR register (0x03)
	if (addr_from_base == PPU_OAMADDR_PORT) {
		data = oamaddr;
	}
	// OAMDATA register
	if (addr_from_base == PPU_OAMDATA_PORT) {
		char *buf = (char*)oam;
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
			ppu_internal.t_register &= 0xFF00;
			ppu_internal.t_register |= data;
			ppu_internal.v_register = ppu_internal.t_register;
		}
		ppu_internal.address_write_latch = !ppu_internal.address_write_latch;
	}
	if (addr_from_base == PPU_DATA_PORT) {
		vbus->writememory(ppu_internal.v_register, data);
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
					ppu_internal.shiftreg_nametable = vbus->readmemory(0x2000 | (ppu_internal.v_register & 0x0FFF));
				}
				else if (((cycle - 1) % 8) == 3) {					
					// load attribute table shift register.
					word attr_address = ppu_internal.v_register & ~0x2000;
					attr_address = 0x03C0 | (attr_address & 0x0C00) | ((attr_address >> 4) & 0x0038) | ((attr_address >> 2) & 0x0007);
					// better fetch the palette entry @ latch mode, finding it later is incredibly difficult.
					word ab = vbus->readmemory(attr_address | 0x2000);
					word paladdr = (ab >> (((((attr_address & 0x001F) >> 1) % 2) << 1) | ((((attr_address & 0x03E0) >> 6) % 2) << 2)) & 0x0003) << 2;					
					ppu_internal.shiftreg_attribute_par = (byte)paladdr;
				}
				else if (((cycle - 1) % 8) == 5) {
					// load pattern table tile low
					ppu_internal.y_shift = (ppu_internal.v_register & 0x7000) >> 12;
					word pattern_address = (ppuctrl.bg_0x1000 << 12) | ((word)ppu_internal.shiftreg_nametable << 4) | ppu_internal.y_shift;
					ppu_internal.shiftregs_pattern_par[0] = vbus->readmemory(pattern_address);
				}
				else if (((cycle - 1) % 8) == 7) {
					// load pattern table tile high
					word pattern_address = (ppuctrl.bg_0x1000 << 12) | ((word)ppu_internal.shiftreg_nametable << 4) | 8 | ppu_internal.y_shift;
					ppu_internal.shiftregs_pattern_par[1] = vbus->readmemory(pattern_address);
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
				if (((cycle - 1) % 8) == 1) {
					byte secaddr = (cycle - 1) / 8;
					memset(&ppu_internal.secoam[secaddr], 0xFF, 4);
					ppu_internal.n = 0;
					ppu_internal.ss = 0;
					ppu_internal.m = 0;
				}
			}
			// sprite eval (cycli 65--256)
			if ((cycle >= 65) && (cycle <= 256)) {				
				if (ppu_internal.n < 64) {
					bool odd = ((cycle - 1) % 2) > 0;
					if (!odd) {
						ppu_internal.secoamb = (byte *)&ppu_internal.secoam[ppu_internal.ss];
						if (ppu_internal.m == 0) {
							// step 1.
							if (ppu_internal.ss < 8) {
								ppu_internal.secoamb[ppu_internal.m] = oam[ppu_internal.n].y;
								ppu_internal.m++;
							}
							else {
								if ((oam[ppu_internal.n].y >= scanline + 1) && (oam[ppu_internal.n].y <= scanline + 1 + (ppuctrl.sprites_8x16 ? 16 : 8))) {
									ppustatus.sprite_overflow = true;
								}
							}
						}
						else {
							if ((oam[ppu_internal.n].y >= scanline + 1) && (oam[ppu_internal.n].y <= scanline + 1 + (ppuctrl.sprites_8x16 ? 16 : 8))) {
								// is in scanline. copy it.
								ppu_internal.secoamb[ppu_internal.m] = ((byte *)&oam[ppu_internal.n])[ppu_internal.m];
								if (ppu_internal.m == 3) {
									// copy complete increment n and ss and reset m to 0
									ppu_internal.m = 0;
									ppu_internal.n++;
									ppu_internal.ss++;
								}
							}
							else {
								// it is not. ignore it further and increment n.
								ppu_internal.n++;
								ppu_internal.m = 0;
							}
						}					
					}
				}
			}
			// sprite loading.
			if ((cycle >= 257) && (cycle <= 320)) {
				byte cs = (cycle - 1) % 8;
				if (cs == 1) {

				}
				else if (cs == 3) {
					// load x and attr.
					ppu_internal.shiftreg_spr_counter[ppu_internal.ss] = ppu_internal.secoam[ppu_internal.ss].x;
					ppu_internal.shiftreg_spr_latch[ppu_internal.ss] = ppu_internal.secoam[ppu_internal.ss].attr;
				}
				else if (cs == 5) {
					// load pattern table tile low
					//word pattern_address = (ppuctrl.sprites_0x1000 << 12) | ((word)ppu_internal.secoam[ppu_internal.ss].tile 
					//ppu_internal.shiftreg_spr_pattern_lo[ppu_internal.ss] = vbus->readmemory(pattern_address);
				}
				else if (cs == 7) {

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
					byte y = ((ppu_internal.v_register & 0x03E0) >> 5);
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
		// BG rendering cycles.
		byte bg = 0x00;
		if ((scanline >= 0) && (scanline <= 239)) {
			if ((cycle >= 4) && (cycle <= 259)) {
				if (ppumask.showbg) {
					// load new data from the parallel registers.
					if ((cycle - 4) % 8 == 0) {
						ppu_internal.shiftregs_pattern[0] |= ppu_internal.shiftregs_pattern_par[0];
						ppu_internal.shiftregs_pattern[1] |= ppu_internal.shiftregs_pattern_par[1];
					}

					// get the pixel.	
					word pix_mux = 0x8000 >> ppu_internal.x_shift;
					byte palentry = (((ppu_internal.shiftregs_pattern[1] & pix_mux) > 0) << 1) |
						((ppu_internal.shiftregs_pattern[0] & pix_mux) > 0);

					// shift the registers.
					ppu_internal.shiftregs_pattern[0] <<= 1;
					ppu_internal.shiftregs_pattern[1] <<= 1;

					// draw to framebuffer?
					framebuffer[scanline * 256 + beam] = (palentry * 64) | (palentry * 64) << 8;
					framebuffer[scanline * 256 + beam] |= (ppumask.emp_blu ? 0x0100 : 0) |
						(ppumask.emp_grn ? 0x0200 : 0) |
						(ppumask.emp_red ? 0x0400 : 0);
				}
				else framebuffer[scanline * 256 + beam] = 0xFFFF;
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
			if (cycle == 1) {
				// reset flags.
				ppustatus.vblank = false;
				ppustatus.sprite_0_hit = false;
				ppustatus.sprite_overflow = false;
			}
			if ((cycle >= 280) && (cycle <= 304)) {
				// reload v register from parts of the t register.
				if (ppumask.showbg | ppumask.showspr) {
					ppu_internal.v_register = (ppu_internal.v_register & ~0x7BE0) | (ppu_internal.t_register & 0x7BE0);
				}
			}
		}

		// Scanline..
		if (cycle == 339) {
			scanline++;
			if (scanline == 30) ppustatus.sprite_0_hit = true;	// fake it for now.
			beam = 0;
			// framebuffer done.
			if (scanline == 240) {
				frameready = true;
			}
			if (scanline == 262) {
				scanline = 0;
				if (ppu_internal.odd_even_frame) cycle = 340;
				ppu_internal.odd_even_frame = !ppu_internal.odd_even_frame;
			}
		}
		cycle++; 
		if (cycle == 341) {			
			cycle = 0;
		}
	}
	return ticks;	// assume ticks in = ticks out.
}

void	ppu::set_char_rom(bus_device *vdata) {
	// first remove any linked rom/rams!
	// it is destructive.
	vbus->removedevice_select_base(0x0000);
	// reregister
	vbus->registerdevice(vdata);
}

void	ppu::configure_vertical_mirror() {
	vram->resetpins_to_default();
}

void	ppu::configure_horizontal_mirror() {
	vram->resetpins_to_default();
	vram->swappins(10, 11);
	vram->groundpin(10);
}

void	ppu::dma(byte *data, bool is_output) {
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

ppu::~ppu() {
}

// PPU RAM
ppuram::ppuram() {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "PPU mainram 2k");
	ram = (byte *)malloc(0x800);
	devicestart = 0x2000;
	deviceend = 0x3EFF;
	devicemask = 0x27FF;	 // per default ppu has 2k of RAM mirrored to 4k of address-space.
}

ppuram::~ppuram() {
	free(ram);
}

void	ppuram::write(int addr, int addr_from_base, byte data) {
	ram[addr_from_base] = data;
}

byte	ppuram::read(int addr, int addr_from_base) {
	return ram[addr_from_base];
}

// PPU PAL RAM
ppu_pal_ram::ppu_pal_ram() {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "PPU palette RAM 32 bytes");
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

byte ppu_pal_ram::read(int addr, int addr_from_base) {
	if (addr_from_base == 0x04) return ram[0x00];
	if (addr_from_base == 0x08) return ram[0x00];
	if (addr_from_base == 0x0C) return ram[0x00];
	return ram[pal_addr_compute(addr_from_base)];	
}

