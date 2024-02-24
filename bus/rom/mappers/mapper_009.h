/*
	
		Mapper 009/010 (MMC2/MMC4)
		(c) 2024 P. Santing


*/

#pragma once

#include "../rom.h"

struct mmc2_state {
	byte	prg_bnk;
	byte	chr_fd_1_bnk;
	byte	chr_fd_2_bnk;
	byte	chr_fe_1_bnk;
	byte	chr_fe_2_bnk;
	bool	horimirror;
};

class mmc2_vrom;

class mmc2_rom : public rom {
private:
	byte	*prgram6000;
	byte	*prg8000;
	mmc2_state state;
	mmc2_vrom	*vrom = nullptr;
public:
	bool			mmc4mode = false;
	mmc2_rom();
	~mmc2_rom();
	virtual byte	read(int addr, int addr_from_base, bool onlyread = false);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual void	set_rom_data(byte *data, std::size_t size);
	void			link_vrom(mmc2_vrom *);
	void			update_banks();
	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, std::size_t size);
};

class mmc2_vrom : public vrom {
private:
	byte	*chrfd0000;
	byte	*chrfe0000;
	byte	*chrfd1000;
	byte	*chrfe1000;
	bool	l0fe = false;
	bool	l1fe = false;
public:
	mmc2_vrom();
	void			update_banks(mmc2_state *state);
	virtual	byte	read(int addr, int addr_from_base, bool onlyread = false);
	virtual			void set_rom_data(byte *data, std::size_t size);
};
