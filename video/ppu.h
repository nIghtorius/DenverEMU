/*

	ppu.h:
		Base Class for the NES Picture Processing Unit.
		also known the graphics adapter of the NES.
	
*/

#pragma once
#include "../bus/bus.h"	// it is a bus device.
#include <functional>

#define		PPU_PPUCTRL_PORT				0x00
#define		PPU_PPUMASK_PORT				0x01
#define		PPU_PPUSTATUS_PORT				0x02
#define		PPU_OAMADDR_PORT				0x03
#define		PPU_OAMDATA_PORT				0x04
#define		PPU_SCROLL_PORT					0x05
#define		PPU_ADDRESS_PORT				0x06
#define		PPU_DATA_PORT					0x07

#define		PPU_BASENAMETABLE				0x03
#define		PPU_VRAM_INCREMENT_32BYTES		0x04
#define		PPU_SPRITE_TABLE_0X1000			0x08
#define		PPU_BG_TABLE_0X1000				0x10
#define		PPU_SPRITE_8X16					0x20
#define		PPU_MASTER_MODE					0x40
#define		PPU_DO_NMI						0x80

#define		PPU_GREYSCALE					0x01
#define		PPU_BG_L8P						0x02
#define		PPU_SPR_L8P						0x04
#define		PPU_SHOW_BG						0x08
#define		PPU_SHOW_SPR					0x10
#define		PPU_EMP_RED						0x20
#define		PPU_EMP_GREEN					0x40
#define		PPU_EMP_BLUE					0x80

#define		PPU_SPR_OVERFLOW				0x20
#define		PPU_SPR_0HIT					0x40
#define		PPU_VBLANK						0x80

#define		OAM_SPR_ATTR_PALETTE			0x03
#define		OAM_SPR_ATTR_UNDEFINED			0x1C
#define		OAM_SPR_INTERNAL_ONLY_SPR0F		0x10
#define		OAM_SPR_PRIORITY				0x20
#define		OAM_SPR_ATTR_FLIP_HOR			0x40
#define		OAM_SPR_ATTR_FLIP_VER			0x80

#pragma pack (push, 1)
struct oamentry {
	byte	y;
	byte	tile;
	byte	attr;
	byte	x;
};
#pragma pack(pop)

struct ppu_ctrl_register {
	byte	base_name_table;
	bool	increment_32_bytes;
	bool	sprites_0x1000;
	bool	bg_0x1000;
	bool	sprites_8x16;
	bool	master_mode;
	bool	do_nmi;
};

struct ppu_mask_register {
	bool	grayscale;
	bool	bg8lt;
	bool	spr8lt;
	bool	showbg;
	bool	showspr;
	bool	emp_red;
	bool	emp_grn;
	bool	emp_blu;
};

struct ppu_status_register {
	bool	sprite_overflow;
	bool	sprite_0_hit;
	bool	vblank;
};

struct ppu_render_state {
	bool		address_write_latch;
	byte		x_shift;
	byte		y_shift;
	word		t_register; // internal temporary register
	word		v_register; // https://wiki.nesdev.com/w/index.php/PPU_rendering @ PPU address bus contents
	byte		shiftregs_pattern_latch;
	word		shiftregs_pattern[2];		// pattern shift registers.
	word		shiftreg_attribute[2];		// attribute shift registers.
	byte		shiftreg_attribute_latch;	//
	oamentry	secoam[8];					// secondary OAM
	byte*		secoamb;
	byte		n;							// index into primary OAM.
	byte		m;							// copy byte.
	byte		sn;							// index into secondary OAM.
	byte		spr_pix;					// pixel buffer for sprite. rendering depends on showspr
	byte		spr_pix_pal;
	byte		buffer_oam_read;
	bool		oam_clearing;
	bool		oam_copy;
	bool		oam_evald;					// true when OAM eval is complete n == 64
	byte		shiftreg_spr_pattern_lo[8];	// sprite pattern shift register (8)
	byte		shiftreg_spr_pattern_hi[8];	// sprite pattern shift register (8)
	byte		shiftreg_spr_latch[8];		// sprite latch shift register (attr data)
	byte		shiftreg_spr_counter[8];	// sprite counter shift register (x positions)
	byte		shiftreg_nametable;
	bool		odd_even_frame;
};

class ppuram : public bus_device {
private:
public:
	byte*	ram;
	ppuram();
	~ppuram();
	byte	read(int addr, int addr_from_base);
	void	write(int addr, int addr_from_base, byte data);
};

class ppu_pal_ram : public bus_device {
private:
	byte * ram;
	int		pal_addr_compute(int addr);
public:
	ppu_pal_ram();
	~ppu_pal_ram();
	byte	read(int addr, int addr_from_base);
	void	write(int addr, int addr_from_base, byte data);
};

class ppu : public bus_device {
public:
	ppu_ctrl_register		ppuctrl;
	ppu_mask_register		ppumask;
	ppu_status_register		ppustatus;
	ppu_render_state		ppu_internal;
	byte					latch;
	word					ppuaddr;	
	int						scanline;	// virtual scanline we are on.
	int						beam;		// beam.
	int						cycle;		// ppu cycle 
	word*					framebuffer;	// holds the frame buffer (16 bits, because PPU is 9 bits)
	bool					frameready;
	byte					prt2007buffer;	

public:
	bus						vbus;		// vbus = videobus.
	ppu_pal_ram				vpal;
	ppuram					vram;
	oamentry				oam[64];
	byte					oamaddr;

	int						ppu_cycles_per_frame;

	// callback function (in order to speed things up)
	std::function<void()>	callback = nullptr;

	ppu();
	~ppu();
	byte					read(int addr, int addr_from_base);
	void					write(int addr, int addr_from_base, byte data);
	int						rundevice(int ticks);
	void					set_char_rom(bus_device *vdata);
	void					configure_vertical_mirror();
	void					configure_horizontal_mirror();
	void					dma(byte *data, bool is_output, bool started);
	void*					getFrameBuffer();
	bool					isFrameReady();	// when true next call will be false unless the frame is ready again.
	void					reset();
	void 					write_state_dump (const char *filename);
};
