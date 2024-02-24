/*

	Mapper 024/026 Konami VRC6
	(c) 2023 P. Santing

	Load cart type(s) 0x18 & 0x1A

	Does not contain the VRC6 audio emulation code. 
	this will be in /audio/expansion/vrc6.*

*/

#pragma once

#include "../rom.h"
#include "../audio/expansion/vrc6.h"

struct vrc6_state {
	// mode
	byte	mode = 0;
	// prg rom
	byte	prg1_bank;	// switchable 16KB
	byte	prg2_bank;	// switchable  8kB
	// prg ram
	byte	r[8];		// char regs.
	byte	mirror;
	// irq
	byte	irq_mode;
	byte	irq_latch_reload;
	byte	irq_latch;
	bool	irq_enabled;
	bool	irq_start_after_ack;
	bool	prg_ram_enable;
	bool	chr10;
	int		prescaler;
};

class vrc6vrom;

class vrc6rom : public rom {
private:
	vrc6vrom	*charrom = nullptr;
	vrc6_state	state;
	char		*ram;
	void		setbanks();
	byte		*prg_8000;
	byte		*prg_c000;
	byte		*prg_e000;
public:
	bool		mapper_026 = false;		// switches between mapper 24/26 mode.
	vrc6audio	*audiochip = nullptr;
	vrc6rom();
	~vrc6rom();
	virtual byte read(int addr, int addr_from_base, bool onlyread = false);
	virtual void write(int addr, int addr_from_base, byte data);
	virtual int	 rundevice(int ticks);
	void		 link_vrom(vrc6vrom *rom);
	virtual	void reset();
	virtual void set_rom_data(byte *data, std::size_t size);
	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, std::size_t size);
};

class vrc6vrom : public vrom {
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
	vrc6vrom();
	void		setbanks(vrc6_state *state);
	virtual void set_rom_data(byte *data, std::size_t size);
	virtual byte read(int addr, int addr_from_base, bool onlyread = false);
};