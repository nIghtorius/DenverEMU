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
#include "../../package/2a03.h"

// EXP audio
#define		NSF_EXP_VRC6			0x01
#define		NSF_EXP_VRC7			0x02
#define		NSF_EXP_MMC5			0x08
#define		NSF_EXP_NAMCO163		0x10
#define		NSF_EXP_SUNSOFT			0x20

// uFirmware
const byte nsfufirm[] = {
  0x0c, 0x30, 0x4a, 0x30, 0x4d, 0x30, 0x3c, 0x30, 0x40, 0x30, 0x4b, 0x30, 0x78, 0xa2, 0x28, 0x8e,
  0x17, 0x40, 0xa6, 0xff, 0x9a, 0xe8, 0x4c, 0x3b, 0x30, 0x8e, 0x01, 0x20, 0x2c, 0x02, 0x20, 0x2c,
  0x02, 0x20, 0x10, 0xfb, 0x2c, 0x02, 0x20, 0x10, 0xfb, 0xad, 0x02, 0x20, 0xa9, 0x00, 0x8d, 0xd5,
  0x07, 0xa9, 0xd0, 0x8d, 0x05, 0x20, 0xa9, 0x1e, 0x8d, 0x01, 0x20, 0xa9, 0x00, 0xa2, 0x00, 0x20,
  0x00, 0x80, 0xa9, 0x00, 0x8d, 0x00, 0x20, 0x4c, 0x47, 0x30, 0x20, 0x00, 0x80, 0x40
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

public:
	nsf_state state;
	byte		*ram = nullptr;
	byte		*prg[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };	// program banks.
	int			nmi_trig_cycles = 0;

	// expansion audio.
	vrc6audio	*vrc6exp = nullptr;
	sunsoftaudio *sunexp = nullptr;
	namco163audio *namexp = nullptr;
	vrc7audio* vrc7exp = nullptr;
	mmc5audio* mmc5exp = nullptr;

	package_2a03 *n2a03 = nullptr;

	uint64_t		timestarted = 0;

	nsfrom();
	~nsfrom();
	virtual	byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual	void	write(const int addr, const int addr_from_base, const byte data);
	virtual int		rundevice(const int ticks);
	virtual void	set_rom_data(byte *data, const std::size_t size);
	void			initialize(const byte song);
};