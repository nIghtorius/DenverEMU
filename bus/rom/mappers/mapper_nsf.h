/*

	Mapper for NSF
	(c) 2023 P. Santing

	Maps NSF files to the program space of the emulated CPU.

*/

#pragma once

#include "../rom.h"
#include "../../audio/expansion/vrc6.h"
#include "../../audio/expansion/sunsoft5b.h"
#include "../../package/2a03.h"

// EXP audio
#define		NSF_EXP_VRC6			0x01
#define		NSF_EXP_SUNSOFT			0x20

// firmware
static const byte nsfufirm[] = {
  0x0c, 0x08, 0x4a, 0x08, 0x4d, 0x08, 0x3c, 0x08, 0x40, 0x08, 0x4b, 0x08, 0x78, 0xa2, 0x28, 0x8e,
  0x17, 0x40, 0xa6, 0xff, 0x9a, 0xe8, 0x8e, 0x00, 0x20, 0x8e, 0x01, 0x20, 0x2c, 0x02, 0x20, 0x2c,
  0x02, 0x20, 0x10, 0xfb, 0x2c, 0x02, 0x20, 0x10, 0xfb, 0xad, 0x02, 0x20, 0xa9, 0x00, 0x8d, 0xd5,
  0x07, 0xa9, 0xd0, 0x8d, 0x05, 0x20, 0xa9, 0x1e, 0x8d, 0x01, 0x20, 0xa9, 0x00, 0xa2, 0x00, 0x20,
  0x00, 0x80, 0xa9, 0x80, 0x8d, 0x00, 0x20, 0x4c, 0x47, 0x08, 0x20, 0x00, 0x80, 0x40
};

#pragma pack (push, 1)
struct nsf_ufirmware_header {
	word	reset;
	word	nmi;
	word	irq;
	word	trackselect;
	word	init;
	word	play;
};
#pragma pack (pop)


struct nsf_state {
	byte	banks[8];
	word	init;
	word	play;
	word	load;

	int		numsongs;
	int		currentsong;

	word	irq_vector;
	word	nmi_vector;
	word	res_vector;
};


class nsfrom : public rom {
private:
	byte		ufirm[128];	// uFirmware space (only 128bytes)
public:
	nsf_state state;
	byte		*ram;
	byte		*prg[8];	// program banks.
	// expansion audio.
	vrc6audio	*vrc6exp = nullptr;
	sunsoftaudio *sunexp = nullptr;

	package_2a03 *n2a03 = nullptr;

	nsfrom();
	~nsfrom();
	virtual	byte	read(int addr, int addr_from_base, bool onlyread = false);
	virtual	void	write(int addr, int addr_from_base, byte data);
	virtual int		rundevice(int ticks);
	void			initialize(byte song);
};