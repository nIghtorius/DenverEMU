/*

	Mapper 021, 022, 023 and 025
	(c) P. Santing

	Load cart type(s) 21, 22, 23 and 25 (VRC2a-c, VRC4a-f)

*/

#pragma once

#include "../rom.h"

#define		VRC24_COMPAT_MAPPER_21		0x01
#define		VRC24_COMPAT_MAPPER_23		0x02
#define		VRC24_COMPAT_MAPPER_25		0x03


// machine state vrc2-4(a-c,a-f)
struct vrc2_4_state {
	bool	ram_enable;
	bool	sm_fix_8000;
	byte	prgbank0;
	byte	prgbank1;
	byte	mirror;
	word	c[8];
	byte	irq_latch;
	byte	irq_latch_reload;
	byte	irq_mode;
	bool	irq_enabled;
	bool	irq_start_after_ack;
	int		prescaler;
};

class vrc2_4_vrom;

class vrc2_4_rom : public rom {
private:
	vrc2_4_state	state;
	byte			*ram;
	byte		*prg_8000;
	byte		*prg_a000;
	byte		*prg_c000;
	byte		*prg_e000;
	vrc2_4_vrom	*charrom = nullptr;
public:
	bool		vrc2_mode;
	byte		submapper = 0;
	byte		run_as_mapper = 0;
	int			compability_mode = 0;	// no compatibilty. 
	vrc2_4_rom();
	~vrc2_4_rom();
	virtual	byte read(int addr, int addr_from_base, bool onlyread = false);
	virtual void write(int addr, int addr_from_base, byte data);
	virtual int  rundevice(int ticks);
	void		 link_vrom(vrc2_4_vrom *rom);
	virtual void reset();
	virtual void set_rom_data(byte *data, std::size_t size);
	word		 recompute_addr(word addr);
	void		 setbanks();
	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, std::size_t size);
	virtual void	set_debug_data();
};

class vrc2_4_vrom : public vrom {
private:
	byte		*chr_0000;
	byte		*chr_0400;
	byte		*chr_0800;
	byte		*chr_0c00;
	byte		*chr_1000;
	byte		*chr_1400;
	byte		*chr_1800;
	byte		*chr_1c00;
public:
	vrc2_4_vrom();
	void		setbanks(vrc2_4_state *state);
	virtual void set_rom_data(byte *data, std::size_t size);
	virtual byte read(int addr, int addr_from_base, bool onlyread = false);
};
