/*

	cart.h	(c) 2023 P. Santing.	
	Implements cartridge and loading.
	NES headers.

*/

#pragma once

#include "../bus.h"
#include "../../video/ppu.h"
#include "../../audio/audio.h"
#include <cstdint>
#include <iostream>
#include <istream>

// mappers.
#include "../rom/rom.h"

// expansions.
#include "../../audio/expansion/vrc6.h"


// end of mappers.

// BITS in header.
#define		INES_F1_MIRRORING						0x01
#define		INES_F1_BATTERY							0x02
#define		INES_F1_TRAINER							0x04
#define		INES_F1_IGNORE_MIRRORING				0x08
#define		INES_F1_LO_NIB_MAPPER_NO				0xF0

#define		INES_F2_VS_UNISYSTEM					0x01
#define		INES_F2_PLAYCHOICE						0x02
#define		INES_F2_NES20_BITPATTERN				0x0C
#define		INES_F2_HI_NIB_MAPPER_NO				0xF0
#define		INES_F2_ARCHAIC_ID						0x04

#define		INES_F3_PROGRAM_RAM						0xFF

#define		INES_F4_TVSYSTEM						0x01

#define		INES_F5_TVSYSTEM						0x03
#define		INES_F5_PRG_RAM_PRESENT					0x10
#define		INES_F5_BUS_CONFLICT					0x20


// NSF bits header.

#pragma pack (push, 1)
struct nes_header_raw {
	std::uint32_t		header_signature;
	byte				program_blocks;
	byte				char_blocks;	 // 0 = RAM
	byte				flags1;
	byte				flags2;
	byte				flags3;
	byte				flags4;
	byte				flags5;
	byte				reserved[21];
};

struct nsf_header_raw {
	std::uint32_t		header_signature;
	byte				header2;
	byte				version;
	byte				total_songs;
	byte				start_song;
	word				load_address;
	word				init_address;
	word				play_address;
	char				songname[32];
	char				artist[32];
	char				copyright[32];
	word				playspeed_ntsc;
	byte				bank_init[8];
	word				playspeed_pal;
	byte				palntscbits;
	byte				expansion_audio;
	byte				nsf2_reserved;
	byte				program_size[3];
};
#pragma pack (pop)

struct nes_header_data {
	// <nes 2.0
	bool				valid_nes_header;		// false is signature mismatch.
	std::size_t			programsize;
	std::size_t			charsize;
	bool				mirror_vertical;//false is horizontal
	bool				has_battery;
	bool				has_trainer;
	bool				no_mirroring; // 4screen
	int					mapper;
	bool				vs_unisystem;
	bool				has_playchoice;
	bool				has_nes20;

	// nes 2.0 specific.
	int					program_ram_size;		// 8KB blocks of program ram.
	bool				is_pal;					// false is ntsc.
	byte				tv_system;
	bool				has_prg_ram;
	bool				bus_conflicts;
};


class cartridge {
private:
	ppu		*l_ppu;
	bus		*m_bus;
	audio_player *m_aud;
	vrc6audio *vrc6exp = nullptr;
	void	readstream(std::istream &stream, ppu *ppu_device, bus *mainbus, audio_player *audbus);
	bool	readstream_nsf(std::istream &stream, ppu *ppu_device, bus *mainbus, audio_player *audbus);
public:
	bool	is_valid;
	rom		*program;
	vrom	*character;
	bool	nsf_mode = false;
	char	songname[32];
	char	artist[32];
	char	copyright[32];
	cartridge(const char *filename, ppu *ppu_device, bus *mainbus, audio_player *audbus);
	cartridge(std::istream &stream, ppu *ppu_device, bus *mainbus, audio_player *audbus);
	~cartridge();
};
