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
	n163vrom* charrom = nullptr;
	n163_state state;
	char* ram;
	void	setbanks();
	byte* prg_8000 = nullptr;
	byte* prg_a000 = nullptr;
	byte* prg_c000 = nullptr;
	byte* prg_e000 = nullptr;
public:
	namco163audio* audiochip = nullptr;
	n163rom();
	~n163rom();
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void write(const int addr, const int addr_from_base, const byte data);
	virtual int rundevice(const int ticks);
	void	link_vrom(n163vrom* rom);
	virtual void reset();
	virtual void set_rom_data(byte* data, const std::size_t size);
	virtual batterybackedram* get_battery_backed_ram();
	virtual void set_battery_backed_ram(byte* data, const std::size_t size);
};

class n163vrom : public vrom {
private:
	byte* chr_0000 = nullptr;
	byte* chr_0400 = nullptr;
	byte* chr_0800 = nullptr;
	byte* chr_0c00 = nullptr;
	byte* chr_1000 = nullptr;
	byte* chr_1400 = nullptr;
	byte* chr_1800 = nullptr;
	byte* chr_1c00 = nullptr;
	byte* chr_2000 = nullptr;	// this makes it interesting. probably should override the default "ciram" with this mapper.
	byte* chr_2400 = nullptr;
	byte* chr_2800 = nullptr;
	byte* chr_2c00 = nullptr;
	char* vram;
public:
	n163vrom();
	~n163vrom();
	void	setbanks(n163_state* state);
	virtual void set_rom_data(byte* data, const std::size_t size);
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void write(const int addr, const int addr_from_base, const byte data);
};
