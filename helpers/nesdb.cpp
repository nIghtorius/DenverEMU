#include "nesdb.h"
#include <fstream>
#include <cstdint>
#include <iostream>

struct CRC32_s
{
	void generate_table(uint32_t(&table)[256])
	{
		uint32_t polynomial = 0xEDB88320;
		for (uint32_t i = 0; i < 256; i++)
		{
			uint32_t c = i;
			for (size_t j = 0; j < 8; j++)
			{
				if (c & 1) {
					c = polynomial ^ (c >> 1);
				}
				else {
					c >>= 1;
				}
			}
			table[i] = c;
		}
	}

	uint32_t update(uint32_t(&table)[256], uint32_t initial, const void* buf, const size_t len)
	{
		uint32_t c = initial ^ 0xFFFFFFFF;
		const uint8_t* u = static_cast<const uint8_t*>(buf);
		for (size_t i = 0; i < len; ++i)
		{
			c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
		}
		return c ^ 0xFFFFFFFF;
	}
};

class CRC32
{
private:
	uint32_t table[256];
	CRC32_s crc32_s;
	uint32_t initial;
public:
	CRC32()
		: initial(0)
	{
		crc32_s.generate_table(table);
	}

	void Update(const uint8_t* buf, const size_t len)
	{
		initial = crc32_s.update(table, initial, (const void*)buf, len);
	}

	uint32_t GetValue() const
	{
		return initial;
	}
};

nesdb::nesdb(const char * filename) {
	// check file nes20db.bin
	std::ifstream	nesdb_file(filename, std::ios_base::binary);
	if (!nesdb_file.good()) {
		std::cout << "No NES 2.0 DB found (" << filename << "), no db loaded.\n";
		return;
	}
	// load database?
	nesdb_file.read((char*)&dbstats, sizeof(db_header));
	
	// validate database file.
	if (dbstats.signature != 0x4244324e) {
		std::cout << "DB is invalid format\n";
		return;
	}

	std::cout << filename << " found, db has " << std::dec << dbstats.entries << " entries.\n";
	std::cout << "nes 20 db version " << std::dec << dbstats.version << "\n";
	std::cout << "nes 20 db size is " << std::dec << (int)dbstats.entries * sizeof(db_game) << " bytes.\n";

	// load db entries.
	int db_size = (int)dbstats.entries * sizeof(db_game);

	// allocate memory.
	entries = new db_game[db_size];

	// load data.
	nesdb_file.seekg(sizeof(db_header), std::ios_base::beg);
	nesdb_file.read((char*)entries, db_size);

	// close file.
	nesdb_file.close();

	// set db to loaded state.
	db_loaded = true;
}

nesdb::~nesdb() {
	if (entries != nullptr)
		delete entries;
}

int		nesdb::in_db(const void* prg, const std::size_t size) {
	// compute crc32 of prg and checks if it is in the db.
	CRC32 crc;
	crc.Update((const unsigned char*)prg, size);

	std::cout << "Finding PRG CRC: " << std::hex << (int)crc.GetValue() << "\n";

	// check DB if found.
	for (int i = 0; i < dbstats.entries; i++) {
		if (entries[i].prgrom_crc32 == crc.GetValue()) {
			std::cout << "Game found in DB: " << entries[i].gamename << "\n";
			return i;
		}
	}

	return -1;	// default is none found.
}

db_game nesdb::get_game_id(const int id) const {
	return entries[id];
}
