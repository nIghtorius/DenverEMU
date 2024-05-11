/*

	Mapper 001 MMC
	(c) 2023 P. Santing

	Load cart type 0x01 (MMC)

	// rom specifics.
	// load state 0x8000 = Switchable, defaults to first.
	//			  0xC000 = Fixed, Last 16kB block of ROM data.

	// Back is selected by writing to 0x8000-0xFFFF
	// 7 bit  0
	// --------
	// xxxxpPPP		- UxROM uses PPP, UOROM uses pPPP

	// vrom specifics
	// No VROM has VRAM

*/

#pragma once

#include "../rom.h"		// also includes the basic 8kB vram class.

#define	MMC1_CONTROL					0x00
#define MMC1_CHRBANK0					0x01
#define MMC1_CHRBANK1					0x02
#define MMC1_PRGBANK					0x03

#define MMC1_PRG_32K_MODE				0x00
#define MMC1_PRG_32K_MODE2				0x01
#define MMC1_PRG_FIRSTBANK_FIX_8000		0x02
#define MMC1_PRG_LASTBANK_FIX_C000		0x03

#define MMC1_CHR_8K_MODE				0x00
#define MMC1_CHR_4K_MODE				0x01

#define MMC1_MIRROR_ONESCRN_LOWER		0x00
#define MMC1_MIRROR_ONESCRN_UPPER		0x01
#define MMC1_MIRROR_VERTICAL			0x02
#define MMC1_MIRROR_HORIZONTAL			0x03

// machine state mmc1 roms
struct mmc1_state {
	byte	shift_reg;
	byte	shift_cnt;
	byte	mode;
	byte	control;

	byte	mirroring;
	byte	prg_bank_mode;
	byte	chr_bank_mode;

	byte	chrbank0;
	byte	chrbank1;
	byte	prgbank;
};

// vrom class.
class mmc1_vrom : public vrom {
private:
	byte			*chr0000 = nullptr;
	byte			*chr1000 = nullptr;
	byte			*ram = nullptr;
	bool			ram_mode = false;
public:
	mmc1_vrom();
	~mmc1_vrom();
	virtual byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual void	set_rom_data(byte *data, const std::size_t size);
	void			update_banks(mmc1_state &state);
	void			is_ram(const bool enable);
};

// rom class.
class mmc1_rom : public rom {
private:
	mmc1_vrom		*charrom = nullptr;
	mmc1_state		state;
	char			*mmc1ram = nullptr;
	byte			*prg8000 = nullptr;
	byte			*prgC000 = nullptr;
public:
	mmc1_rom();
	~mmc1_rom();
	virtual byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual void	set_rom_data(byte *data, const std::size_t size);
	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, const std::size_t size);
	void			link_vrom(mmc1_vrom *vrom);
	void			update_control();
	void			update_banks();
};