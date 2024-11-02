/*

	Mapper for NSF
	(c) 2023 P. Santing

	Maps NSF files to the program space of the emulated CPU.

*/

#pragma once

#include "../rom.h"
#include "../../audio/expansion/vrc6.h"
#include "../../audio/expansion/vrc7.h"
#include "../../audio/expansion/sunsoft5b.h"
#include "../../audio/expansion/namco163.h"
#include "../../audio/expansion/mmc5.h"
#include "../../audio/expansion/fds_audio.h"
#include "../../package/2a03.h"

// EXP audio
#define		NSF_EXP_VRC6			0x01
#define		NSF_EXP_VRC7			0x02
#define		NSF_EXP_FDS				0x04
#define		NSF_EXP_MMC5			0x08
#define		NSF_EXP_NAMCO163		0x10
#define		NSF_EXP_SUNSOFT			0x20

// uFirmware
const byte nsfufirm[] = {
  0x0c, 0x30, 0x2a, 0x30, 0x2d, 0x30, 0x1c, 0x30, 0x20, 0x30, 0x2b, 0x30, 0x78, 0xa2, 0x40, 0x8e,
  0x17, 0x40, 0xa2, 0xff, 0x9a, 0xe8, 0xa9, 0x00, 0x8d, 0x00, 0x20, 0xa9, 0x00, 0xa2, 0x00, 0x20,
  0x00, 0x80, 0xa9, 0xff, 0x8d, 0x00, 0x30, 0x4c, 0x27, 0x30, 0x20, 0x00, 0x80, 0x40
};

#pragma pack (push, 1)
struct nsf_ufirmware_header {
	word	reset = 0;
	word	nmi = 0;
	word	irq = 0;
	word	trackselect = 0;
	word	init = 0;
	word	play = 0;
};
#pragma pack (pop)


struct nsf_state {
	byte	banks[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	byte	sbanks[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	word	init = 0;
	word	play = 0;
	word	load = 0;

	int		numsongs = 0;
	int		currentsong = 0;

	word	irq_vector = 0;
	word	nmi_vector = 0;
	word	res_vector = 0;
};


class nsfrom : public rom {
private:
	byte		ufirm[128];	// uFirmware space (only 128bytes)
	int			tickcount = 0;
	uint64_t	total_cpu_ticks = 0;

public:
	nsf_state state;
	byte		*ram = nullptr;
	byte		*prg[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };	// program banks.
	int			nmi_trig_cycles = 0;
	bool		nmi_enabled = false;

	// expansion audio.
	vrc6audio	*vrc6exp = nullptr;
	sunsoftaudio *sunexp = nullptr;
	namco163audio *namexp = nullptr;
	vrc7audio* vrc7exp = nullptr;
	mmc5audio* mmc5exp = nullptr;
	fdsaudio* fdsexp = nullptr;

	package_2a03 *n2a03 = nullptr;

	nsfrom();
	~nsfrom();
	virtual	byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual	void	write(const int addr, const int addr_from_base, const byte data);
	virtual int		rundevice(const int ticks);
	virtual void	set_rom_data(byte *data, const std::size_t size);
	void			initialize(const byte song);
	virtual void	set_debug_data();
	uint64_t		return_time_in_ms() const;
};