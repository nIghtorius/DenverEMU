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
	byte	cmdreg;
	byte	c[8];		// char banks.
	bool	prg_ram_enable;
	bool	ramrom_select;	// true = prgram, false = prgrom
	byte	prg0_select;	// 0x6000-7FFF
	byte	p[3];		// prg bank 1-3
	byte	mirroring;
	bool	irq_enable;
	bool	irq_counter_enable;
	word	irq_counter;
};

class fme7vrom;

class fme7rom : public rom {
private:
	fme7vrom		*charrom;
	fme7_state		state;
	byte*			ram;		// it can have 256kB of it.
	void			setbanks();
	const int		prgsize = 262144;
	byte			*prg_6000;		// can be rom or ram.
	byte			*prg_8000;
	byte			*prg_a000;
	byte			*prg_c000;
	byte			*prg_e000;
public:
	fme7rom();
	~fme7rom();
	sunsoftaudio	*audiochip = nullptr;
	virtual byte	read(int addr, int addr_from_base, bool onlyread = false);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual int		rundevice(int ticks);	
	virtual void	reset();
	virtual void	set_rom_data(byte *data, std::size_t size);
	void			link_vrom(fme7vrom *rom);
};

class fme7vrom : public vrom {
private:
	byte			*chr_0000;
	byte			*chr_0400;
	byte			*chr_0800;
	byte			*chr_0c00;
	byte			*chr_1000;
	byte			*chr_1400;
	byte			*chr_1800;
	byte			*chr_1c00;
public:
	fme7vrom();
	~fme7vrom();
	void			setbanks(fme7_state *state);
	virtual	void	set_rom_data(byte *data, std::size_t size);
	virtual byte	read(int addr, int addr_from_base, bool onlyread = false);
};