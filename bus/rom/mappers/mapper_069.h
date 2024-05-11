/*

	Sunsoft		FME-7 mapper (Mapper 069)
	(c) 2003 P. Santing
	Denver Project.

*/

#pragma once

#include "../rom.h"

// audio expansion
#include "../../../audio/expansion/sunsoft5b.h"

#define		FME7_CMD_CHRBANK0		0x00
#define		FME7_CMD_CHRBANK1		0x01
#define		FME7_CMD_CHRBANK2		0x02
#define		FME7_CMD_CHRBANK3		0x03
#define		FME7_CMD_CHRBANK4		0x04
#define		FME7_CMD_CHRBANK5		0x05
#define		FME7_CMD_CHRBANK6		0x06
#define		FME7_CMD_CHRBANK7		0x07
#define		FME7_CMD_PRGBANK0		0x08
#define		FME7_CMD_PRGBANK1		0x09
#define		FME7_CMD_PRGBANK2		0x0A
#define		FME7_CMD_PRGBANK3		0x0B
#define		FME7_CMD_MIRRORING		0x0C
#define		FME7_CMD_IRQCTRL		0x0D
#define		FME7_CMD_IRQCLO			0x0E
#define		FME7_CMD_IRQCHI			0x0F

struct fme7_state {
	byte	cmdreg = 0;
	byte	c[8] = { 0, 0, 0, 0, 0, 0, 0 ,0 };		// char banks.
	bool	prg_ram_enable = false;
	bool	ramrom_select = false;	// true = prgram, false = prgrom
	byte	prg0_select = 0;	// 0x6000-7FFF
	byte	p[3] = { 0, 0, 0 };		// prg bank 1-3
	byte	mirroring = 0;
	bool	irq_enable = false;
	bool	irq_counter_enable = false;
	word	irq_counter = 0;
};

class fme7vrom;

class fme7rom : public rom {
private:
	fme7vrom		*charrom = nullptr;
	fme7_state		state;
	byte*			ram = nullptr;		// it can have 256kB of it.
	void			setbanks();
	const int		prgsize = 262144;
	byte			*prg_6000 = nullptr;		// can be rom or ram.
	byte			*prg_8000 = nullptr;
	byte			*prg_a000 = nullptr;
	byte			*prg_c000 = nullptr;
	byte			*prg_e000 = nullptr;
public:
	fme7rom();
	~fme7rom();
	sunsoftaudio	*audiochip = nullptr;
	virtual byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual int		rundevice(const int ticks);
	virtual void	reset();
	virtual void	set_rom_data(byte *data, const std::size_t size);
	void			link_vrom(fme7vrom *rom);
};

class fme7vrom : public vrom {
private:
	byte			*chr_0000 = nullptr;
	byte			*chr_0400 = nullptr;
	byte			*chr_0800 = nullptr;
	byte			*chr_0c00 = nullptr;
	byte			*chr_1000 = nullptr;
	byte			*chr_1400 = nullptr;
	byte			*chr_1800 = nullptr;
	byte			*chr_1c00 = nullptr;
public:
	fme7vrom();
	~fme7vrom();
	void			setbanks(fme7_state *state);
	virtual	void	set_rom_data(byte *data, const std::size_t size);
	virtual byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
};