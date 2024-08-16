#pragma once

/*

	This handles the binary DB for NES header data based on CRC32/SHA/SUM16
	A separate utility is required to convert nes20db.xml to the binary file which denver requires.
	Denver normally will check for the file nes20db.bin in the application directory. If found it will recompute the heeader
	using that data. Otherwise it will use the iNES headers. If the game is not found in the DB. It also will
	use the iNES header ofcourse.

*/

// binary format.

#include <cstdlib>
#include <cstdint>

#define	NESDB_HMIRROR		0x00
#define NESDB_VMIRROR		0x01

#pragma pack (push, 1)
struct db_header {
	std::uint32_t	signature;				// expects 4244324e
	std::uint32_t	version;				// version of file.
	std::uint32_t	entries;				// amount of game entries contained in db file.
};

struct db_game {
	char	gamename[128];
	std::uint32_t	prgrom_size;
	std::uint32_t	prgrom_crc32;
	std::uint32_t	chrrom_size;
	std::uint32_t	chrrom_crc32;
	std::uint32_t	charram_size;
	std::uint32_t	charnvram_size;
	std::uint32_t	prgram_size;
	std::uint32_t	prgnvram_size;
	std::uint32_t	rom_size;
	std::uint32_t	rom_crc32;
	std::uint16_t	mapper;
	std::uint8_t	submapper;
	std::uint8_t	mirroring; // 0 -- H, 1 -- V
	std::uint8_t	has_battery;
	// below is included, but probably ignored for now.
	std::uint8_t	consoletype;
	std::uint8_t	region;
	std::uint8_t	controller_expansion;
};

#pragma pack (pop)

class nesdb {
private:
	db_header	dbstats;
	db_game*	entries;
public:
	bool		db_loaded = false;
	nesdb();
	~nesdb();
	int			in_db(const void* prg, std::size_t size);
	db_game		get_game_id(int id);
};
