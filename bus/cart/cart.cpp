#include "cart.h"
#include <fstream>
#include <iostream>

// mappers
#include "../rom/mappers/mapper_001.h"
#include "../rom/mappers/mapper_002.h"
#include "../rom/mappers/mapper_003.h"
#include "../rom/mappers/mapper_004.h"
#include "../rom/mappers/mapper_024026.h"

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

	if ((ines.flags2 & INES_F2_NES20_BITPATTERN) == INES_F2_ARCHAIC_ID)
		mapper &= 0x0F;		// shave off high nibble (archiac nes file, only 4 bit support for mapper)


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

void	cartridge::readstream(std::istream &nesfile, ppu *ppu_device, bus *mainbus, audio_player *audbus) {

	program = NULL;
	character = NULL;

	nes_header_raw nes_hdr;
	nesfile.read((char *)&nes_hdr, 16);

	nes_header_data nes = parse_nes_header(nes_hdr);

	is_valid = nes.valid_nes_header;

	// do nothing is header is not valid. 
	if (!nes.valid_nes_header) {
		return;
	}

	std::cout << "cartridge is valid." << std::endl;
	std::cout << "Program size: " << std::dec << (int)nes.programsize << " bytes.." << std::endl;
	if (nes.charsize > 0) {
		std::cout << "Charrom size: " << (int)nes.charsize << " bytes.." << std::endl;
	}
	else {
		std::cout << "Cartridge contains VRAM" << std::endl;
	}

	// load program data
	void * program_data = malloc(nes.programsize);
	void * char_data = malloc(16);

	if (!program_data || !char_data) {
		std::cout << "Failed to reserve memory for cartridge file." << std::endl;
		return;
	}

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
			std::cout << "Cartridge uses vertical mirroring" << std::endl;
			ppu_device->configure_vertical_mirror();
		}
		else {
			std::cout << "Cartridge uses horizontal mirroring" << std::endl;
			ppu_device->configure_horizontal_mirror();
		}
	}

	if (nes.has_battery) std::cout << "Cartridge has battery pack" << std::endl;
	if (nes.has_prg_ram) std::cout << "Cartridge has program ram" << std::endl;
	if (nes.has_trainer) std::cout << "Cartridge has trainer prg" << std::endl;

	std::cout << "Cartridge mapper is: " << nes.mapper << std::endl;

	vram *charram;

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
			charram = new vram();
			character = charram;
		}
		break;
	case 1:
		// MMC1_ROM
		program = new mmc1_rom();
		character = new mmc1_vrom();	//mmc1 vrom also emulated vram.
		// mmc1 linking.
		reinterpret_cast<mmc1_rom*>(program)->link_vrom(reinterpret_cast<mmc1_vrom*>(character));
		reinterpret_cast<mmc1_vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		program->set_rom_data((byte *)program_data, nes.programsize);
		if (has_char_data) {
			character->set_rom_data((byte *)char_data, nes.charsize);
		}
		else {
			reinterpret_cast<mmc1_vrom*>(character)->is_ram(true);
		}
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
			charram = new vram();
			character = charram;
		}
		break;
	case 3:
		// CNROM
		program = new cnrom();
		program->set_rom_data((byte *)program_data, nes.programsize);
		if (has_char_data) {
			character = new cnvrom();
			character->set_rom_data((byte *)char_data, nes.charsize);
			reinterpret_cast<cnrom*>(program)->link_vrom(reinterpret_cast<cnvrom*>(character));
		}
		else {
			charram = new vram();
			character = charram;	// should not happen.
		}
		break;
	case 4:
		// MMC3
		program = new mmc3_rom();
		character = new mmc3_vrom();
		// mmc3 linking.
		reinterpret_cast<mmc3_rom*>(program)->link_vrom(reinterpret_cast<mmc3_vrom*>(character));
		reinterpret_cast<mmc3_vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		program->set_rom_data((byte *)program_data, nes.programsize);
		if (has_char_data) {
			character->set_rom_data((byte *)char_data, nes.charsize);
		}
		else {
			// RAM based MMC3?
			charram = new vram();
			character = charram;			
		}
		break;
	case 24:
		// VRC6a
		program = new vrc6rom();
		character = new vrc6vrom();
		vrc6exp = new vrc6audio();
		vrc6exp->vrc6_mapper_026 = false;
		audbus->register_audible_device(vrc6exp);
		mainbus->registerdevice(vrc6exp);
		// vrc6 linking.
		reinterpret_cast<vrc6rom*>(program)->link_vrom(reinterpret_cast<vrc6vrom*>(character));
		reinterpret_cast<vrc6vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		reinterpret_cast<vrc6rom*>(program)->mapper_026 = false;
		reinterpret_cast<vrc6rom*>(program)->audiochip = vrc6exp;
		if (has_char_data) {
			character->set_rom_data((byte *)char_data, nes.charsize);
		}
		program->set_rom_data((byte *)program_data, nes.programsize);		
		break;
	case 26:
		// VRC6b
		program = new vrc6rom();
		character = new vrc6vrom();
		vrc6exp = new vrc6audio();
		vrc6exp->vrc6_mapper_026 = true;
		audbus->register_audible_device(vrc6exp);
		mainbus->registerdevice(vrc6exp);
		// vrc6 linking.
		reinterpret_cast<vrc6rom*>(program)->link_vrom(reinterpret_cast<vrc6vrom*>(character));
		reinterpret_cast<vrc6vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		reinterpret_cast<vrc6rom*>(program)->mapper_026 = true;
		reinterpret_cast<vrc6rom*>(program)->audiochip = vrc6exp;
		if (has_char_data) {
			character->set_rom_data((byte *)char_data, nes.charsize);
		}		
		program->set_rom_data((byte *)program_data, nes.programsize);
		break;
	default:
		std::cout << "Mapper is unknown to me" << std::endl;
		break;
	}

	// link roms..
	mainbus->registerdevice(program);
	ppu_device->set_char_rom(character);

	// reset vector.
	std::cout << "ROM Reset vector: 0x" << std::hex << (int)mainbus->readmemory_as_word(0xfffc) << std::endl;


	// link busses.
	m_aud = audbus;
	m_bus = mainbus;
	l_ppu = ppu_device;
}

cartridge::cartridge(std::istream &stream, ppu *ppu_device, bus *mainbus, audio_player *audbus) {
	std::cout << "Loading cartridge from memory 0x" << std::hex << (std::uint64_t)&stream << std::endl;
	readstream(stream, ppu_device, mainbus, audbus);
}

cartridge::cartridge(const char *filename, ppu *ppu_device, bus *mainbus, audio_player *audbus) {
	std::cout << "Loading cartridge: " << filename << std::endl;
	// load & parse NES file.
	std::ifstream	nesfile;
	nesfile.open(filename, std::ios::binary | std::ios::in);
	readstream(nesfile, ppu_device, mainbus, audbus);
	nesfile.close();
}

cartridge::~cartridge() {
	if (m_aud != NULL) {
		if (vrc6exp) {
			m_aud->unregister_audible_device(vrc6exp);
			m_bus->removedevice_select_base(vrc6exp->devicestart);
		}
	}
	if (m_bus != NULL) {
		if (program != NULL)
		m_bus->removedevice_select_base(program->devicestart);
	}
	if (l_ppu != NULL) {
		l_ppu->set_char_rom(NULL);
	}
	if (program != NULL) delete program;
	if (character != NULL) delete character;
}