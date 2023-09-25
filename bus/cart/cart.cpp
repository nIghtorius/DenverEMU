#include "cart.h"
#include <fstream>

// implementation.

// base functions
nes_header_data		parse_nes_header(nes_header_raw &ines) {
	// 0x1A54454E
	nes_header_data	data;
	data.valid_nes_header = (ines.header_signature == (std::uint32_t)0x1A53454E);
	data.programsize = (int)ines.program_blocks * 16384;
	data.charsize = (int)ines.char_blocks * 8192;
	data.mirror_vertical = (ines.flags1 & INES_F1_MIRRORING) > 0;
	data.has_battery = (ines.flags1 & INES_F1_BATTERY) > 0;
	data.has_trainer = (ines.flags1 & INES_F1_TRAINER) > 0;
	data.no_mirroring = (ines.flags1 & INES_F1_IGNORE_MIRRORING) > 0;

	byte mapper = ((ines.flags1 & INES_F1_LO_NIB_MAPPER_NO) >> 4) |
		((ines.flags2 & INES_F2_HI_NIB_MAPPER_NO));

	data.mapper = (int)mapper;
	data.vs_unisystem = (ines.flags2 & INES_F2_VS_UNISYSTEM) > 0;
	data.has_playchoice = (ines.flags2 & INES_F2_PLAYCHOICE) > 0;
	data.has_nes20 = (ines.flags2 & INES_F2_NES20_BITPATTERN) == INES_F2_NES20_BITPATTERN;

	// nes 2.0 formats.. (these can be ignored when has_nes20 is false)
	data.program_ram_size = ines.flags3 * 8192;
	data.is_pal = (ines.flags4 & INES_F4_TVSYSTEM) > 0;
	data.tv_system = (ines.flags5 & INES_F5_TVSYSTEM);
	data.has_prg_ram = (ines.flags5 & INES_F5_PRG_RAM_PRESENT) > 0;
	data.bus_conflicts = (ines.flags5 & INES_F5_BUS_CONFLICT) > 0;

	return data;
}

// classes

cartridge::cartridge(const char *filename, ppu *ppu_device, bus *mainbus) {
	charram = new vram();
	
	// load & parse NES file.
	std::ifstream	nesfile;
	nesfile.open(filename, std::ios::binary | std::ios::in);
	
	nes_header_raw nes_hdr;
	nesfile.read((char *)&nes_hdr, 16);

	nes_header_data nes = parse_nes_header(nes_hdr);

	is_valid = nes.valid_nes_header;

	// do nothing is header is not valid. 
	if (!nes.valid_nes_header) {
		return;
	}

	// load program data
	void * program_data = malloc(nes.programsize);
	void * char_data = malloc(16);
	nesfile.read((char*)program_data, nes.programsize);

	// load character data.
	bool	has_char_data = (nes.charsize > 0);
	if (has_char_data) {
		free(char_data);		// i know it is stupid otherwise it will not compile.
		char_data = malloc(nes.charsize);
		nesfile.read((char*)char_data, nes.charsize);
	}

	// configure mirroring.
	if (!nes.no_mirroring) {
		if (nes.mirror_vertical) {
			ppu_device->configure_vertical_mirror();
		}
		else {
			ppu_device->configure_horizontal_mirror();
		}
	}

	// initialize loader.
	switch (nes.mapper) {
		case 0: 
			// NROM.
			program = new rom();
			program->set_rom_data((byte *)program_data, nes.programsize);
			if (has_char_data) {
				character = new vrom();
				character->set_rom_data((byte *)char_data, nes.charsize);
			}
			else {
				character = charram;
			}
			break;
		case 1:
			// MMC1_ROM
			program = new mmc1_rom();
			program->set_rom_data((byte *)program_data, nes.programsize);
			character = new mmc1_vrom();	//mmc1 vrom also emulated vram.
			if (has_char_data) {
				character->set_rom_data((byte *)char_data, nes.charsize);
			}
			else {
				reinterpret_cast<mmc1_vrom*>(character)->is_ram(true);
			}
			// mmc1 linking.
			reinterpret_cast<mmc1_rom*>(program)->link_vrom(reinterpret_cast<mmc1_vrom*>(character));
			reinterpret_cast<mmc1_vrom*>(character)->link_ppu_bus(&ppu_device->vram);
			break;
		case 2:
			// UXROM
			program = new uxrom();
			program->set_rom_data((byte *)program_data, nes.programsize);
			if (has_char_data) {
				character = new vrom();
				character->set_rom_data((byte *)char_data, nes.charsize);
			}
			else {
				character = charram;
			}
			break;
	}

	// link roms..
	mainbus->registerdevice(program);
	ppu_device->set_char_rom(character);

	// link busses.
	m_bus = mainbus;
	l_ppu = ppu_device;
}

cartridge::~cartridge() {
	if (m_bus != NULL) {
		if (program != NULL)
		m_bus->removedevice_select_base(program->devicestart);
	}
	if (l_ppu != NULL) {
		l_ppu->set_char_rom(NULL);
	}
	delete charram;
	if (program != NULL) delete program;
	if (character != NULL) delete character;
}
