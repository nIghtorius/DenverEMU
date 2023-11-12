#include "cart.h"
#include <fstream>
#include <iostream>

// mappers
#include "../rom/mappers/mapper_001.h"
#include "../rom/mappers/mapper_002.h"
#include "../rom/mappers/mapper_003.h"
#include "../rom/mappers/mapper_004.h"
#include "../rom/mappers/mapper_024026.h"
#include "../rom/mappers/mapper_021-22-23-25.h"
#include "../rom/mappers/mapper_069.h"

// NSF
#include "../rom/mappers/mapper_nsf.h"

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
	data.ext_mapper = mapper & (ines.flags3 & INES_F3_NES20_MAPPER_HIHI) << 12;
	data.submapper = (ines.flags3 & INES_F3_NES20_SUBMAPPER) >> 4;

	return data;
}

// classes
bool	cartridge::readstream_nsf(std::istream &nsffile, ppu *ppu_device, bus *mainbus, audio_player *audbus) {
	program = NULL;
	character = NULL;	// will not be used.

	nsf_header_raw nsf_hdr;
	nsffile.seekg(0, std::ios_base::beg);
	nsffile.read((char*)&nsf_hdr, sizeof(nsf_header_raw));

	bool is_valid = (nsf_hdr.header_signature == (std::uint32_t)0x4D53454E) && (nsf_hdr.header2 == 0x1A);
	
	if (!is_valid) {
		return false;
	}

	std::cout << "Cartridge is NSF file..\n";
	std::cout << "Songname  : " << nsf_hdr.songname << "\n";
	std::cout << "Artist    : " << nsf_hdr.artist << "\n";
	std::cout << "Copyright : " << nsf_hdr.copyright << "\n";
	std::cout << "# of song : " << std::dec << (int)nsf_hdr.total_songs << "\n";
	std::cout << "L_ADDR    : 0x" << std::hex << (int)nsf_hdr.load_address << "\n";
	std::cout << "I_ADDR    : 0x" << std::hex << (int)nsf_hdr.init_address << "\n";
	std::cout << "P_ADDR    : 0x" << std::hex << (int)nsf_hdr.play_address << "\n";
	std::cout << "BANKS     : ";
	for (int i = 0; i < 8; i++) {
		std::cout << "0x" << std::hex << (int)nsf_hdr.bank_init[i] << " ";
	}
	std::cout << "\n";

	int	program_size = nsf_hdr.program_size[0] << 16 | nsf_hdr.program_size[1] << 8 | nsf_hdr.program_size[2];

	std::cout << "PRG_SIZE  : " << std::dec << (int)program_size << " bytes.. (header based)\n";

	// program_size in header is wonky. So we are to ignore that.
	nsffile.seekg(0, std::ios_base::end);
	program_size = (int)nsffile.tellg() - sizeof(nsf_header_raw);
	nsffile.seekg(sizeof(nsf_header_raw), std::ios_base::beg);

	std::cout << "PRG_SIZEc : " << std::dec << (int)program_size << " bytes.. (computed)\n";

	// loading NSF is tricky. because load address is not always 0x8000
	// we do not support < 0x8000, check this first.
	if (nsf_hdr.load_address < 0x8000) {
		std::cout << "We cannot load NSF file with load address below 0x8000\n";
		return false;
	}

	int	prealloc = nsf_hdr.load_address - 0x8000;

	// load program data.
	byte * program_data = (byte*)malloc(prealloc + program_size);

	nsffile.read((char*)&program_data[prealloc], program_size);

	// NSF is loaded.
	// build a NSF cartridge.
	program = new nsfrom();
	nsfrom	*nsf_rom = reinterpret_cast<nsfrom*>(program);

	// configure NSF cartridge.
	nsf_rom->state.numsongs = nsf_hdr.total_songs;
	nsf_rom->state.currentsong = 1;
	for (int i = 0; i < 8; i++) nsf_rom->state.banks[i] = nsf_hdr.bank_init[i];
	nsf_rom->state.init = nsf_hdr.init_address;
	nsf_rom->state.play = nsf_hdr.play_address;
	nsf_rom->set_rom_data(program_data, program_size + prealloc);

	// expansions.
	if (nsf_hdr.expansion_audio & NSF_EXP_VRC6) {
		vrc6exp = new vrc6audio();
		vrc6exp->vrc6_mapper_026 = false;
		audbus->register_audible_device(vrc6exp);
		mainbus->registerdevice(vrc6exp);
		nsf_rom->vrc6exp = vrc6exp;
	}
	if (nsf_hdr.expansion_audio & NSF_EXP_SUNSOFT) {
		sunexp = new sunsoftaudio();
		audbus->register_audible_device(sunexp);
		mainbus->registerdevice(sunexp);
		nsf_rom->sunexp = sunexp;
	}	

	// add to mainbus
	mainbus->registerdevice(nsf_rom);

	// initialize cart for song #1 (0)
	nsf_rom->initialize(0);

	// link busses.
	m_aud = audbus;
	m_bus = mainbus;
	l_ppu = ppu_device;

	// copy meta data.
	memcpy(songname, nsf_hdr.songname, 32);
	memcpy(artist, nsf_hdr.artist, 32);
	memcpy(copyright, nsf_hdr.copyright, 32);

	nsf_mode = true;

	return true;
}

void	cartridge::readstream(std::istream &nesfile, ppu *ppu_device, bus *mainbus, audio_player *audbus) {

	program = NULL;
	character = NULL;

	nsf_mode = false;

	nes_header_raw nes_hdr;
	nesfile.read((char *)&nes_hdr, 16);

	nes_header_data nes = parse_nes_header(nes_hdr);

	is_valid = nes.valid_nes_header;

	// do nothing is header is not valid. 
	if (!nes.valid_nes_header) {
		// before giving up. try first NSF.
		if (!readstream_nsf(nesfile, ppu_device, mainbus, audbus))
			std::cout << "Cartridge is invalid format\n";
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
	case 26:
		// VRC6b
		program = new vrc6rom();
		character = new vrc6vrom();
		vrc6exp = new vrc6audio();
		vrc6exp->vrc6_mapper_026 = (nes.mapper == 26);
		audbus->register_audible_device(vrc6exp);
		mainbus->registerdevice(vrc6exp);
		// vrc6 linking.
		reinterpret_cast<vrc6rom*>(program)->link_vrom(reinterpret_cast<vrc6vrom*>(character));
		reinterpret_cast<vrc6vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		reinterpret_cast<vrc6rom*>(program)->mapper_026 = (nes.mapper == 26);
		reinterpret_cast<vrc6rom*>(program)->audiochip = vrc6exp;
		if (has_char_data) {
			character->set_rom_data((byte *)char_data, nes.charsize);
		}		
		program->set_rom_data((byte *)program_data, nes.programsize);
		break;
	case 69:
		// Sunsoft FME-7
		program = new fme7rom();
		character = new fme7vrom();
		sunexp = new sunsoftaudio();
		audbus->register_audible_device(sunexp);
		mainbus->registerdevice(sunexp);
		// fme7 linking.
		reinterpret_cast<fme7rom*>(program)->link_vrom(reinterpret_cast<fme7vrom*>(character));
		reinterpret_cast<fme7vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		reinterpret_cast<fme7rom*>(program)->audiochip = sunexp;
		if (has_char_data) {
			character->set_rom_data((byte *)char_data, nes.charsize);
		}
		program->set_rom_data((byte *)program_data, nes.programsize);
		break;
	case 21:
	case 22:
	case 23:
	case 25:
		std::cout << "Mapper 21-23, 25 detected. Enabling NES2.0 Header support\n";
		std::cout << "Has NES 2.0 header? " << (nes.has_nes20 ? "Yes" : "No") << "\n";
		if (nes.has_nes20) {
			std::cout << "NES 2.0 Extended Mapper type: " << (int)nes.ext_mapper << "\n";
			std::cout << "NES 2.0 Submapper: " << (int)nes.submapper<< "\n";
			program = new vrc2_4_rom();
			character = new vrc2_4_vrom();
			bool vrc2 = false;		// assume VRC4 unless conditions down below are met.
			// enable vrc mode for the follow combinations.
			if ((nes.mapper == 22) && (nes.submapper == 0)) vrc2 = true;
			if ((nes.mapper == 23) && (nes.submapper == 3)) vrc2 = true;
			if ((nes.mapper == 25) && (nes.submapper == 3)) vrc2 = true;
			reinterpret_cast<vrc2_4_rom*>(program)->link_vrom(reinterpret_cast<vrc2_4_vrom*>(character));
			reinterpret_cast<vrc2_4_rom*>(program)->vrc2_mode = vrc2;
			reinterpret_cast<vrc2_4_rom*>(program)->run_as_mapper = nes.mapper;
			reinterpret_cast<vrc2_4_rom*>(program)->submapper = nes.submapper;		// important in NES2.0 mode.
			reinterpret_cast<vrc2_4_rom*>(program)->compability_mode = 0;	// native mode, no compat.
			reinterpret_cast<vrc2_4_vrom*>(character)->link_ppu_bus(&ppu_device->vram);
			if (has_char_data) {
				character->set_rom_data((byte *)char_data, nes.charsize);
			}
			program->set_rom_data((byte *)program_data, nes.programsize);
		}
		else {
			std::cout << "No specific mapper information, compatibility mode enabled.\n";
			program = new vrc2_4_rom();
			character = new vrc2_4_vrom();
			int compat;
			// switch compatibility layers.
			if (nes.mapper == 21) {
				// emulate VRC4a, VRC4c (detect these chips during emulation)
				compat = VRC24_COMPAT_MAPPER_21;
			}
			if (nes.mapper == 23) {
				// emulate vrc2b+vrc4, VRC4e
				compat = VRC24_COMPAT_MAPPER_23;
			}
			if (nes.mapper == 25) {
				// emulate VRC2c+VRC4b, VRC4d
				compat = VRC24_COMPAT_MAPPER_25;
			}
			// VRC4 linking.
			reinterpret_cast<vrc2_4_rom*>(program)->link_vrom(reinterpret_cast<vrc2_4_vrom*>(character));
			reinterpret_cast<vrc2_4_rom*>(program)->vrc2_mode = false;
			reinterpret_cast<vrc2_4_rom*>(program)->run_as_mapper = nes.mapper;
			reinterpret_cast<vrc2_4_rom*>(program)->submapper = 0;
			reinterpret_cast<vrc2_4_rom*>(program)->compability_mode = compat;
			reinterpret_cast<vrc2_4_vrom*>(character)->link_ppu_bus(&ppu_device->vram);
			if (has_char_data) {
				character->set_rom_data((byte *)char_data, nes.charsize);
			}			
			program->set_rom_data((byte *)program_data, nes.programsize);
		}
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
			delete vrc6exp;
		}
		if (sunexp) {
			m_aud->unregister_audible_device(sunexp);
			m_bus->removedevice_select_base(sunexp->devicestart);
			delete sunexp;
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