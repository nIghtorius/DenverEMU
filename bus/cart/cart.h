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
#include "../../audio/expansion/sunsoft5b.h"
#include "../../audio/expansion/namco163.h"
#include "../../audio/expansion/vrc7.h"
#include "../../audio/expansion/mmc5.h"
#include "../../audio/expansion/fds_audio.h"

// db
#include "../../helpers/nesdb.h"


// end of mappers.

// BITS in header.
#define		INES_F1_MIRRORING						0x01
#define		INES_F1_BATTERY							0x02
#define		INES_F1_TRAINER							0x04
#define		INES_F1_IGNORE_MIRRORING				0x08
#define		INES_F1_LO_NIB_MAPPER_NO				0xF0

#define		INES_F2_VS_UNISYSTEM					0x01
#define		INES_F2_PLAYCHOICE						0x02
#define		INES_F2_NES20_BITPATTERN				0x08
#define		INES_F2_HI_NIB_MAPPER_NO				0xF0
#define		INES_F2_ARCHAIC_ID						0x04

#define		INES_F3_PROGRAM_RAM						0xFF
#define		INES_F3_NES20_SUBMAPPER					0xF0
#define		INES_F3_NES20_MAPPER_HIHI				0x0F

#define		INES_F4_TVSYSTEM						0x01

#define		INES_F5_TVSYSTEM						0x03
#define		INES_F5_PRG_RAM_PRESENT					0x10
#define		INES_F5_BUS_CONFLICT					0x20

// NSFe chunk ids
#define		NSFE_INFO								0x4F464E49
#define		NSFE_BANK								0x4B4E4142
#define		NSFE_RATE								0x45544152
#define		NSFE_DATA								0x41544144
#define		NSFE_NEND								0x444E454E
#define		NSFE_AUTH								0x68747561
#define		NSFE_TLBL								0x6C626C74
#define		NSFE_TIME								0x656D6974

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

// FDS
struct fds_header {
	std::uint32_t		header_signature;
	byte				disksides;
	byte				reserved[11];
};

// NSFe
struct nsfe_header {
	std::uint32_t		header_signature;
};

struct nsfe_chunk {
	std::uint32_t		length;
	std::uint32_t		chunkid;	
};

struct nsfe_info {
	word				load_address;
	word				init_address;
	word				play_address;
	byte				pal_ntsc_flags;
	byte				expansion_audio;
	byte				total_songs;
	byte				start_song;
};

struct nsfe_bank {
	byte				bank_init[8];
};

struct nsfe_rate {
	word				playspeed_ntsc;
	word				playspeed_pal;
	word				playspeed_dendy;
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
	byte				submapper;
	bool				vs_unisystem;
	bool				has_playchoice;
	bool				has_nes20;

	// nes 2.0 specific.
	int					program_ram_size;		// 8KB blocks of program ram.
	bool				is_pal;					// false is ntsc.
	byte				tv_system;
	bool				has_prg_ram;
	bool				bus_conflicts;
	int					ext_mapper;
};

struct auth_data {
	std::string			title = "<?>";
	std::string			artist = "<?>";
	std::string			copyright = "<?>";
	std::string			ripper = "<?>";
};

class cartridge {
private:
	ppu* l_ppu = nullptr;
	bus		*m_bus = nullptr;
	audio_player *m_aud = nullptr;
	void	readstream(std::istream &stream, ppu *ppu_device, bus *mainbus, audio_player *audbus, const char *orgfilename);
	bool	readstream_nsf(std::istream &stream, ppu *ppu_device, bus *mainbus, audio_player *audbus);
	bool	readstream_nsfe(std::istream& stream, ppu* ppu_device, bus* mainbus, audio_player* audbus);
	bool	readstream_fds(std::istream& stream, ppu* ppu_device, bus* mainbus, audio_player* audbus, const char *orgfilename);
	nesdb* gamedb;

public:

	namco163audio *namexp = nullptr;
	vrc6audio* vrc6exp = nullptr;
	sunsoftaudio* sunexp = nullptr;
	vrc7audio* vrc7exp = nullptr;
	mmc5audio* mmc5exp = nullptr;
	fdsaudio* fdsexp = nullptr;

	bool	is_valid;
	rom*	program = nullptr;
	vrom*	character = nullptr;
	bool	nsf_mode = false;
	bool	high_hz_nsf = false;
	int		nsf_cpu_cycles = 0;

	std::string songname = "<?>";
	std::string artist = "<?>";
	std::string copyright = "<?>";
	std::string ripper = "<?>";
	std::vector<std::string>trackNames;
	std::vector<std::int32_t>trackLengths;

	cartridge(const char *filename, ppu *ppu_device, bus *mainbus, audio_player *audbus, nesdb *db = nullptr);
	cartridge(std::istream &stream, ppu *ppu_device, bus *mainbus, audio_player *audbus, nesdb *db = nullptr);
	~cartridge();
};
