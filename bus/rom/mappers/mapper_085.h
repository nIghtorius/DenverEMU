/*

	Mapper 085 Konami VRC7
	(c) 2024 P. Santing

	Load cart type(s) 0x55

	Does not contain the VRC7 audio emulation core.
	this will be in /audio/expansion/vrc7.*
	
*/

#pragma once

#include "../rom.h"
#include "../audio/expansion/vrc7.h"

struct vrc7_state {
	// banks
	byte prgrom[3] = { 0, 0, 0 };	// prog banks (8kB)
	byte chrrom[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // char banks (1kB)
	// mirror
	byte mirror = 0;
	// irq
	byte irq_mode = 0;
	byte irq_latch_reload = 0;
	byte irq_latch = 0;
	bool irq_enabled = false;
	bool irq_start_after_ack = false;
	int  prescaler = 0;
	// prgram
	bool wram_enabled = false;
};

class vrc7vrom;

class vrc7rom : public rom {
private:
	vrc7vrom* charrom = nullptr;
	vrc7_state	state;
	char*		ram = nullptr;
	void		setbanks();
	byte*		prg_8000 = nullptr;
	byte*		prg_a000 = nullptr;
	byte*		prg_c000 = nullptr;
	byte*		prg_e000 = nullptr;
public:
	vrc7audio* audiochip = nullptr;
	vrc7rom();
	~vrc7rom();
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void write(const int addr, const int addr_from_base, const byte data);
	virtual int rundevice(const int ticks);
	void		link_vrom(vrc7vrom* rom);
	virtual void reset();
	virtual void set_rom_data(byte* data, const std::size_t size);
	virtual batterybackedram* get_battery_backed_ram();
	virtual void set_battery_backed_ram(byte* data, const std::size_t size);
};

class vrc7vrom : public vrom {
private:
	byte* chr_0000 = nullptr;
	byte* chr_0400 = nullptr;
	byte* chr_0800 = nullptr;
	byte* chr_0c00 = nullptr;
	byte* chr_1000 = nullptr;
	byte* chr_1400 = nullptr;
	byte* chr_1800 = nullptr;
	byte* chr_1c00 = nullptr;
	char* vram = nullptr;
public:
	vrc7vrom();
	~vrc7vrom();
	void		setbanks(vrc7_state* state);
	virtual void set_rom_data(byte* data, std::size_t size);
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void write(const int addr, const int addr_from_base, const byte data);
	void switch_vram();
};

