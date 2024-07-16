/*

	VRC3 mapper 073
	(c) 2024 P. Santing

*/

#pragma once

#include "../rom.h"

struct vrc3_state {
	byte prgbank = 0;
	word latch = 0;
	word counter = 0;
	bool irq_enable_ack = false;
	bool irq_enable = false;
	byte mode = 0;
};

class vrc3rom : public rom {
private:
	vrc3_state state;
	byte* prg_8000 = nullptr;
	byte* prg_c000 = nullptr;
	byte* ram;
	void setbanks();
public:
	vrc3rom();
	~vrc3rom();
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void write(const int addr, const int addr_from_base, const byte data);
	virtual void set_rom_data(byte* data, const std::size_t size);
	virtual int	 rundevice(int ticks);
};
