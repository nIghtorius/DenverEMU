/*

	Mapper 004 (MMC3)
	(c) 2023 P. Santing

	Load cart type (0x04) (MMC3)
	
	Also emulates the Mapper 52

*/

#pragma once

#include "../rom.h"
#include "../../../video/ppu.h"

#define		MMC3_BANKUPDATE_REG			0x07
#define		MMC3_BANK_MODE				0x40
#define		MMC3_CHR_A12_INVERSION		0x80
#define		MMC3_HORIZONTAL_MIRRORING	0x01
#define		MMC3_PROGRAM_RAM_ENABLE		0x80
#define		MMC3_PROGRAM_RAM_RO			0x40

// mapper 52 defines.
#define		MP52_PRGBANK				0x01
#define		MP52_PRGBANK_A19			0x06
#define		MP52_PRGCHRA19				0x04
#define		MP52_NOPRG256KB				0x08
#define		MP52_CHRBANK				0x30
#define		MP52_NOCHR256KB				0x40
#define		MP52_LOCK_UNTIL_RESET		0x80

struct mmc3_state {
	// banks.
	byte	bank_update_reg;
	bool	prg_bank_mode;
	bool	chr_a12_inv;

	word	r0, r1, r2, r3, r4, r5, r6, r7;

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

	// mapper 44/52
	byte	ob_register = 0x00;
	bool	mapper_52_mode = false;	
	bool	mapper_44_mode = false;
	word	prgand = 0xff;
	word	chrand = 0xff;
	word	prgor = 0x00;
	word	chror = 0x00;
};

class mmc3_vrom;

class mmc3_rom : public rom {
private:
	byte	*prgram6000;
	byte	*prg8000 = nullptr;
	byte	*prga000 = nullptr;
	byte	*prgc000 = nullptr;
	byte	*prge000 = nullptr;
	mmc3_state	state;
	mmc3_vrom	*vrom = nullptr;
	word	lastppuaddr = 0;
	int		cpu_tk = 0;
	int		lt_a12r = 0;
public:
	ppu		*vbus;	// we need to snoop vbus, because VRAM is an option and we cannot rely on the mmc3_vrom.
	mmc3_rom();
	~mmc3_rom();
	virtual	byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual void	set_rom_data(byte *data, const std::size_t size);
	virtual int		rundevice(const int ticks);
	void			link_vrom(mmc3_vrom *);
	void			write_banks(const byte data);
	void			update_banks();
	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, const std::size_t size); 
	virtual void	set_debug_data();
	void			set_mapper52_mode();
	void			set_mapper44_mode();
	virtual void	reset();
};

class mmc3_vrom : public vrom {
private:
	byte	*chr0000 = nullptr;
	byte	*chr0400 = nullptr;
	byte	*chr0800 = nullptr;
	byte	*chr0c00 = nullptr;
	byte	*chr1000 = nullptr;
	byte	*chr1400 = nullptr;
	byte	*chr1800 = nullptr;
	byte	*chr1c00 = nullptr;
public:
	mmc3_vrom();
	mmc3_rom		*rom = nullptr;
	void			update_banks(mmc3_state *state);
	virtual	byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual	void	write(const int addr, const int addr_from_base, const byte data);
	virtual void	set_rom_data(byte *data, const std::size_t size);
	word			fetch_ppu_addr();
};

