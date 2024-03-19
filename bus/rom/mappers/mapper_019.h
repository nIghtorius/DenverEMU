/*

	Mapper 019 Namco 129/163
	(c) 2024 P. Santing

	Load cart type(s) 0x13

	Does not contain the Namco 163 Expansion Audio Emulation Core.
	this will be in /audio/expansion/namco163.*
	
*/

#pragma once

#include "../rom.h"
#include "../../../audio/expansion/namco163.h"

struct n163_state {
	byte	prgbanks[3];
	byte	chrbanks[12];
	byte	writeprotect;
	bool	write_enable;	// hi-nibble 0xF800-0xFFFF must be 0x4
	word	irq_counter;
	bool	irq_counting = true;
	byte	chr_ram;
};

class n163vrom;

class n163rom : public rom {
private:
	n163vrom* charrom;
	n163_state state;
	char* ram;
	void	setbanks();
	byte* prg_8000;
	byte* prg_a000;
	byte* prg_c000;
	byte* prg_e000;
public:
	namco163audio* audiochip = nullptr;
	n163rom();
	~n163rom();
	virtual byte read(int addr, int addr_from_base, bool onlyread = false);
	virtual void write(int addr, int addr_from_base, byte data);
	virtual int rundevice(int ticks);
	void	link_vrom(n163vrom* rom);
	virtual void reset();
	virtual void set_rom_data(byte* data, std::size_t size);
	virtual batterybackedram* get_battery_backed_ram();
	virtual void set_battery_backed_ram(byte* data, std::size_t size);
};

class n163vrom : public vrom {
private:
	byte* chr_0000;
	byte* chr_0400;
	byte* chr_0800;
	byte* chr_0c00;
	byte* chr_1000;
	byte* chr_1400;
	byte* chr_1800;
	byte* chr_1c00;
	byte* chr_2000;	// this makes it interesting. probably should override the default "ciram" with this mapper.
	byte* chr_2400;
	byte* chr_2800;
	byte* chr_2c00;
	char* vram;
public:
	n163vrom();
	~n163vrom();
	void	setbanks(n163_state* state);
	virtual void set_rom_data(byte* data, std::size_t size);
	virtual byte read(int addr, int addr_from_base, bool onlyread = false);
	virtual void write(int addr, int addr_from_base, byte data);
};
