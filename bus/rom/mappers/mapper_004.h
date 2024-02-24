/*

	Mapper 004 (MMC3)
	(c) 2023 P. Santing

	Load cart type (0x04) (MMC3)
	

*/

#pragma once

#include "../rom.h"

#define		MMC3_BANKUPDATE_REG			0x07
#define		MMC3_BANK_MODE				0x40
#define		MMC3_CHR_A12_INVERSION		0x80
#define		MMC3_HORIZONTAL_MIRRORING	0x01
#define		MMC3_PROGRAM_RAM_ENABLE		0x80
#define		MMC3_PROGRAM_RAM_RO			0x40

struct mmc3_state {
	// banks.
	byte	bank_update_reg;
	bool	prg_bank_mode;
	bool	chr_a12_inv;

	int		r0, r1, r2, r3, r4, r5, r6, r7;

	// mirroring
	bool	do_horizontal_mirroring;

	// prgram
	bool	prg_ram_enable;
	bool	prg_ram_readonly;

	// irq
	byte	irq_latch;
	byte	irq_counter;
	bool	irq_enable;
	bool	irq_reload;
};

class mmc3_vrom;

class mmc3_rom : public rom {
private:
	byte	*prgram6000;
	byte	*prg8000;
	byte	*prga000;
	byte	*prgc000;
	byte	*prge000;
	mmc3_state	state;
	mmc3_vrom	*vrom;
	word	lastppuaddr;
public:
	mmc3_rom();
	~mmc3_rom();
	virtual	byte	read(int addr, int addr_from_base, bool onlyread = false);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual void	set_rom_data(byte *data, std::size_t size);
	virtual int		rundevice(int ticks);
	void			link_vrom(mmc3_vrom *);
	void			write_banks(byte data);
	void			update_banks();
	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, std::size_t size);
};

class mmc3_vrom : public vrom {
private:
	byte	*chr0000;
	byte	*chr0400;
	byte	*chr0800;
	byte	*chr0c00;
	byte	*chr1000;
	byte	*chr1400;
	byte	*chr1800;
	byte	*chr1c00;
	word	ppuaddr;
public:
	mmc3_vrom();
	mmc3_rom		*rom;
	void			update_banks(mmc3_state *state);
	virtual	byte	read(int addr, int addr_from_base, bool onlyread = false);
	virtual	void	write(int addr, int addr_from_base, byte data);
	virtual void	set_rom_data(byte *data, std::size_t size);
	word			fetch_ppu_addr();
};

