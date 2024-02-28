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
	byte prgrom[3];	// prog banks (8kB)
	byte chrrom[8]; // char banks (1kB)
	// mirror
	byte mirror;
	// irq
	byte irq_mode;
	byte irq_latch_reload;
	byte irq_latch;
	bool irq_enabled;
	bool irq_start_after_ack;
	int  prescaler;
	// prgram
	bool wram_enabled;
};

class vrc7vrom;

class vrc7rom : public rom {
private:
	vrc7vrom* charrom = nullptr;
	vrc7_state	state;
	char*		ram;
	void		setbanks();
	byte*		prg_8000;
	byte*		prg_a000;
	byte*		prg_c000;
	byte*		prg_e000;
public:
	vrc7audio* audiochip = nullptr;
	vrc7rom();
	~vrc7rom();
	virtual byte read(int addr, int addr_from_base, bool onlyread = false);
	virtual void write(int addr, int addr_from_base, byte data);
	virtual int rundevice(int ticks);
	void		link_vrom(vrc7vrom* rom);
	virtual void reset();
	virtual void set_rom_data(byte* data, std::size_t size);
	virtual batterybackedram* get_battery_backed_ram();
	virtual void set_battery_backed_ram(byte* data, std::size_t size);
};

class vrc7vrom : public vrom {
private:
	byte* chr_0000;
	byte* chr_0400;
	byte* chr_0800;
	byte* chr_0c00;
	byte* chr_1000;
	byte* chr_1400;
	byte* chr_1800;
	byte* chr_1c00;
	char* vram = nullptr;
public:
	vrc7vrom();
	~vrc7vrom();
	void		setbanks(vrc7_state* state);
	virtual void set_rom_data(byte* data, std::size_t size);
	virtual byte read(int addr, int addr_from_base, bool onlyread = false);
	virtual void write(int addr, int addr_from_base, byte data);
	void switch_vram();
};

