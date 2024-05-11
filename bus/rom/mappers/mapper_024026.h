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
	byte	prg1_bank = 0;	// switchable 16KB
	byte	prg2_bank = 0;	// switchable  8kB
	// prg ram
	byte	r[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };		// char regs.
	byte	mirror = 0;
	// irq
	byte	irq_mode = 0;
	byte	irq_latch_reload = 0;
	byte	irq_latch = 0;
	bool	irq_enabled = false;
	bool	irq_start_after_ack = false;
	bool	prg_ram_enable = false;
	bool	chr10 = false;
	int		prescaler = 0;
};

class vrc6vrom;

class vrc6rom : public rom {
private:
	vrc6vrom	*charrom = nullptr;
	vrc6_state	state;
	char		*ram = nullptr;
	void		setbanks();
	byte		*prg_8000 = nullptr;
	byte		*prg_c000 = nullptr;
	byte		*prg_e000 = nullptr;
public:
	bool		mapper_026 = false;		// switches between mapper 24/26 mode.
	vrc6audio	*audiochip = nullptr;
	vrc6rom();
	~vrc6rom();
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void write(const int addr, const int addr_from_base, const byte data);
	virtual int	 rundevice(const int ticks);
	void		 link_vrom(vrc6vrom *rom);
	virtual	void reset();
	virtual void set_rom_data(byte *data, const std::size_t size);
	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, const std::size_t size);
};

class vrc6vrom : public vrom {
private:
	byte		*chr_0000 = nullptr;
	byte		*chr_0400 = nullptr;
	byte		*chr_0800 = nullptr;
	byte		*chr_0c00 = nullptr;
	byte		*chr_1000 = nullptr;
	byte		*chr_1400 = nullptr;
	byte		*chr_1800 = nullptr;
	byte		*chr_1c00 = nullptr;
public:
	vrc6vrom();
	void		setbanks(vrc6_state *state);
	virtual void set_rom_data(byte *data, const std::size_t size);
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
};