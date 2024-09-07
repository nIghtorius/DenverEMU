#include "cart.h"
#include <fstream>
#include <iostream>

// mappers
#include "../rom/mappers/mapper_001.h"
#include "../rom/mappers/mapper_002.h"
#include "../rom/mappers/mapper_003.h"
#include "../rom/mappers/mapper_004.h"
#include "../rom/mappers/mapper_007.h"
#include "../rom/mappers/mapper_009.h"
#include "../rom/mappers/mapper_024026.h"
#include "../rom/mappers/mapper_021-22-23-25.h"
#include "../rom/mappers/mapper_069.h"
#include "../rom/mappers/mapper_071.h"
#include "../rom/mappers/mapper_073.h"
#include "../rom/mappers/mapper_085.h"
#include "../rom/mappers/mapper_087.h"

// FDS
#include "../rom/disksystem/fds.h"

// NSF
#include "../rom/mappers/mapper_nsf.h"
#include <math.h>

#pragma warning(disable : 4996)

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

// helper functions.
char*	parse_data(std::string &text, char* data) {
	text = "";
	while (*data != 0x00) {
		text += *data;
		data++;
	}
	data++;
	return data;
}

bool	parse_auth_data(auth_data* data, char* rawauth) {
	std::string text;
	char* ptr = rawauth;
	for (int i = 0; i < 4; i++) {
		text = "";
		while (*ptr != 0x00) {
			text += *ptr;
			ptr++;
		}
		ptr++; // next record.
		switch (i) {
		case 0:
			data->title = text;
			break;
		case 1:
			data->artist = text;
			break;
		case 2:
			data->copyright = text;
			break;
		case 3:
			data->ripper = text;
			break;
		}
	}
	return true;
}

void fds_gapping(const byte* data, const std::size_t size, std::vector<uint8_t> &fdsImage) {
	fdsImage.insert(fdsImage.end(), 28300 / 8, 0);
	for (size_t i = 0; i < size;) {
		uint8_t dblocktype = data[i];
		uint32_t blockLength = 1;
		switch (dblocktype) {
		case 1: blockLength = 56; break; // main header.
		case 2: blockLength = 2; break; // file count.
		case 3: blockLength = 16; break; // filename header.
		case 4: blockLength = 1 + data[i - 3] + data[i - 2] * 0x0100; break; // data block.
		default: return;
		}

		if (dblocktype == 0x00) {
			fdsImage.push_back(0x00);
		}
		else {
			fdsImage.push_back(0x80);	// gap end bit.
			fdsImage.insert(fdsImage.end(), &data[i], &data[i] + blockLength);
			// add CRC data.
			fdsImage.push_back(0x4D);
			fdsImage.push_back(0x62);
			fdsImage.insert(fdsImage.end(), 976 / 8, 0);
		}
		i += blockLength;
	}
	return;
}

// classes
bool	cartridge::readstream_fds(std::istream& fdsfile, ppu* ppu_device, bus* mainbus, audio_player* audbus, const char* orgfilename) {
	// we already confirmed header signature at this callpoint. No need to revalidate.
	std::cout << "Loading as FDS file..\n";

	fdsfile.seekg(0, std::ios_base::end);
	std::size_t filesize = fdsfile.tellg();
	fdsfile.seekg(0, std::ios_base::beg);

	// read the FDS file.
	bool has_header = ((filesize - 16) % 65500) == 0;

	// build NSF hardware.
	fds_rom* fdsrom;
	try {
		std::cout << "Initializing FDS emulation..\n";
		program = new fds_rom();
	}
	catch(...) {
		std::cout << "Unable to create FDS virtual hardware.\n";
		return false;
	}

	fdsrom = reinterpret_cast<fds_rom*>(program);

	// create VRAM device.
	character = new vram();

	// link devices
	mainbus->registerdevice(program);
	ppu_device->set_char_rom(character);

	// audio
	fdsexp = new fdsaudio();
	audbus->register_audible_device(fdsexp);
	mainbus->registerdevice(fdsexp);
	fdsrom->expaud = fdsexp;

	// register used devices (linking)
	m_aud = audbus;
	m_bus = mainbus;
	l_ppu = ppu_device;

	// load FDS data.
	if (has_header) {
		fdsfile.seekg(16, std::ios_base::beg);
		filesize -= 16;
	}
	byte* diskdata = (byte*)malloc(filesize);

	if (diskdata == nullptr) {
		std::cout << "Something went wrong loading FDS image\n";
		return false;
	}

	fdsfile.read((char*)diskdata, filesize);

	//std::vector<uint8_t>* gappedData = new std::vector<uint8_t>();	
	
	int sides = (int)filesize / 65500;

	for (int j = 0; j < sides; j++) {
		std::vector<uint8_t>* diskSide = new std::vector<uint8_t>();
		fds_gapping((byte*)&diskdata[j*65500], 65500, *diskSide);
		if ((const int)diskSide->size() != 65500) {
			diskSide->resize(65500);
		}
		fdsrom->add_disk((byte*)&diskSide->begin()[0], (const int)diskSide->size());
		delete diskSide;
	}

	free(diskdata);

	// restore saved data.
	// load .dwd (disk write data) file.
	if ((orgfilename != nullptr) && (program != nullptr)) {
		std::string srmfile;
		srmfile += orgfilename;
		srmfile += ".dwd";
		// we have a generated filename. assign it to the loaded program rom.
		strncpy(program->get_sram_filename(), srmfile.c_str(), SRAM_MAX_FILE_NAME);
		std::ifstream srm(srmfile.c_str(), std::ios::binary | std::ios::in);
		if (srm.good()) {
			// load the file.
			srm.seekg(0, std::ios::end);
			size_t srm_size = srm.tellg();
			srm.seekg(0, std::ios::beg);
			byte* sram = (byte*)malloc(srm_size);
			if (sram != NULL) srm.read((char*)sram, srm_size);
			// update cart.
			if (sram != NULL) program->set_battery_backed_ram(sram, srm_size);
			std::cout << "Disk write data loaded from: " << srmfile.c_str() << "\n";
		}
	}


	fdsrom->set_side(0);
	fdsrom->ppudevice = ppu_device;

	std::cout << "Running FDS image..\n";

	ppu_device->configure_horizontal_mirror();

	return true;
}


bool	cartridge::readstream_nsfe(std::istream& nsffile, ppu* ppu_device, bus* mainbus, audio_player* audbus) {
	program = NULL;
	character = NULL;

	nsffile.seekg(0, std::ios_base::end);
	std::size_t filesize = nsffile.tellg();
	nsffile.seekg(0, std::ios_base::beg);

	nsfe_header nsfe;
	nsffile.read((char*)&nsfe, sizeof(nsfe_header));

	// check ID.
	bool is_valid = (nsfe.header_signature == (std::uint32_t)0x4546534E);
	if (!is_valid) return false;

	int		play_speed = 16666;			// default playspeed 60hz

	// read chucks till eof / NEND chunk.
	nsfe_chunk chunk;

	bool end_of_file = false;
	int readsize = 0;

	// required data
	nsfe_info info;
	void* romdata = nullptr;
	void* tracknames = nullptr;
	void* tracklengths = nullptr;
	int	  nr_lengths = 0;
	nsfe_bank bank;
	nsfe_rate rate;
	auth_data auth;
	void* authdata = nullptr;
	int	prgsize = 0;
	memset(&info, 0, sizeof(nsfe_info));
	memset(&bank, 0, sizeof(nsfe_bank));	// set to all zeroes. the NSF mapper knows how to handle this.

	while (!end_of_file) {
		nsffile.read((char*)&chunk, sizeof(nsfe_chunk));
		end_of_file = (chunk.chunkid == NSFE_NEND) || (nsffile.tellg() + (std::streampos)chunk.length >= (std::streampos)filesize);
		switch (chunk.chunkid) {
		case NSFE_INFO:
			std::cout << "Reading INFO chunk..\n";
			readsize = chunk.length;
			if (readsize > sizeof(nsfe_info)) {
				readsize = sizeof(nsfe_info);
				std::cout << "Warning NSFE_INFO block exceeds NSFE_INFO header block size, truncating..\n";
			}
			// read header.
			nsffile.read((char*) &info, readsize);	// read header data.
			chunk.length -= readsize;
			if (chunk.length > 0) nsffile.seekg(chunk.length, std::ios_base::cur);	// skip the rest.
			break;
		case NSFE_BANK:
			std::cout << "Reading BANK chunk..\n";
			readsize = chunk.length;
			if (readsize > 8) {
				std::cout << "Warning NSFE_BANK block exceeds NSFE_BANK size, truncating..\n";
				readsize = 8;
			}
			// read bank data.
			nsffile.read((char*)&bank, readsize);
			chunk.length -= readsize;
			if (chunk.length > 0) nsffile.seekg(chunk.length, std::ios_base::cur);	// skip.
			break;
		case NSFE_RATE:
			std::cout << "Reading RATE chunk..\n";
			readsize = chunk.length;
			if (readsize > sizeof(nsfe_rate)) {
				readsize = sizeof(nsfe_rate);
				std::cout << "Warning NSFE_RATE block exceeds NSFE_RATE header block size, truncating..\n";
			}
			// read rate header.
			nsffile.read((char*)&rate, readsize);
			// set rate.
			play_speed = rate.playspeed_ntsc;		// we only care for NTSC. PAL / DENDY not supported.. (yet)
			chunk.length -= readsize;
			if (chunk.length > 0) nsffile.seekg(chunk.length, std::ios_base::cur);	// skip.
			break;
		case NSFE_DATA:
			std::cout << "Reading DATA chunk..\n";
			romdata = malloc(chunk.length);
			prgsize = chunk.length;
			if (romdata != nullptr) nsffile.read((char*)romdata, chunk.length);
			break;
		case NSFE_AUTH:
			std::cout << "Reading AUTH chunk..\n";
			authdata = malloc(chunk.length);
			nsffile.read((char*)authdata, chunk.length);
			// process.
			parse_auth_data(&auth, (char*)authdata);
			free(authdata);
			break;
		case NSFE_TLBL:
			std::cout << "Reading TLBL chunk..\n";
			tracknames = malloc(chunk.length);
			nsffile.read((char*)tracknames, chunk.length);
			break;
		case NSFE_TIME:
			std::cout << "Reading TIME chunk..\n";
			tracklengths = malloc(chunk.length);
			nsffile.read((char*)tracklengths, chunk.length);
			nr_lengths = chunk.length / sizeof(std::int32_t);
			break;
		default:
			//std::cout << "Unimplemented chunk.. skipping data\n";
			if (chunk.length > 0) nsffile.seekg(chunk.length, std::ios_base::cur);
			break;
		}
	}
	
	// check that we have all the data we need!
	// what is required?
	//		romdata != null
	//		info.load_address != 0x000
	// that's it.. other data will just filled in with  "defaults"

	if (romdata == nullptr) return false;
	if (info.load_address == 0x0000) return false;

	std::cout << "Cartridge is NSFe file..\n";
	std::cout << "Title           : " << auth.title << "\n";
	std::cout << "Artist          : " << auth.artist << "\n";
	std::cout << "Copyright       : " << auth.copyright << "\n";
	std::cout << "Ripper          : " << auth.ripper << "\n";
	std::cout << "# of song       : " << std::dec << (int)info.total_songs << "\n";
	std::cout << "Starting @ song : " << std::dec << (int)info.start_song << "\n";
	std::cout << "Load address    : " << std::hex << (int)info.load_address << "\n";
	std::cout << "Init address    : " << std::hex << (int)info.init_address << "\n";
	std::cout << "Play address    : " << std::hex << (int)info.play_address << "\n";
	std::cout << "BANKS           : ";
	for (int i = 0; i < 8; i++) {
		std::cout << "0x" << std::hex << (int)bank.bank_init[i] << " ";
	}
	std::cout << "\n";

	// compute NSF NMI trigger speed.
	// cpu cycles are 29780 per audio frame (60hz), lower that number and we increase refreshrate.
	// 16666 = 60.002hz that we know. 16666 = 29780 cycles.
	float cpu_cycles_per_frame = (float)(29780.0 / 16666.0) * (float)play_speed;
	std::cout << "SPEED           : " << std::dec << (int)play_speed;
	std::cout << " (" << (int)(1000000 / play_speed) << " Hz, CPU cycles per audioframe: " << (int)cpu_cycles_per_frame << ")\n";

	// if >100hz enable high_hz_mode. OC'ing cpu 3x to keep up.
	if (((int)(1000000 / play_speed)) > 100) {
		high_hz_nsf = true;
	}
	else high_hz_nsf = false;

	nsf_cpu_cycles = (int)cpu_cycles_per_frame;

	std::cout << "PRG_SIZE        : " << std::dec << (int)prgsize << " bytes.. (header based)\n";

	// we do not support < 0x8000, check this first.
	if (info.load_address < 0x8000) {
		std::cout << "We cannot load NSF file with load address below 0x8000\n";
		free(romdata);
		return false;
	}

	bool bankloading = false;
	for (int i = 0; i < 8; i++) if (bank.bank_init[i] != 0x00) bankloading = true;

	int	prealloc = info.load_address - 0x8000;
	if (bankloading) prealloc &= 0x0FFF;

	// load program data.
	byte* program_data = (byte*)malloc(prealloc + prgsize);

	if (program_data != nullptr) {
		// copy the data to the "final" rom.
		memcpy((void*)&program_data[prealloc], romdata, prgsize);
		// dispose of loaded data.
		free(romdata);
	}
	else return false;	// we could not allocate???

	// NSF is loaded.
	// build a NSF cartridge.
	program = new nsfrom();
	nsfrom* nsf_rom = reinterpret_cast<nsfrom*>(program);

	// configure NSF cartridge.
	nsf_rom->nmi_trig_cycles = (int)floorf(cpu_cycles_per_frame);
	nsf_rom->state.numsongs = info.total_songs;
	nsf_rom->state.currentsong = 1;
	for (int i = 0; i < 8; i++) nsf_rom->state.sbanks[i] = bank.bank_init[i];
	nsf_rom->state.init = info.init_address;
	nsf_rom->state.play = info.play_address;
	nsf_rom->state.load = info.load_address;
	nsf_rom->set_rom_data(program_data, prgsize + prealloc);

	// expansions.
	if (info.expansion_audio & NSF_EXP_VRC6) {
		vrc6exp = new vrc6audio();
		vrc6exp->vrc6_mapper_026 = false;
		audbus->register_audible_device(vrc6exp);
		mainbus->registerdevice(vrc6exp);
		nsf_rom->vrc6exp = vrc6exp;
	}
	if (info.expansion_audio & NSF_EXP_SUNSOFT) {
		sunexp = new sunsoftaudio();
		audbus->register_audible_device(sunexp);
		mainbus->registerdevice(sunexp);
		nsf_rom->sunexp = sunexp;
	}
	if (info.expansion_audio & NSF_EXP_NAMCO163) {
		namexp = new namco163audio();
		audbus->register_audible_device(namexp);
		mainbus->registerdevice(namexp);
		nsf_rom->namexp = namexp;
	}
	if (info.expansion_audio & NSF_EXP_VRC7) {
		vrc7exp = new vrc7audio();
		audbus->register_audible_device(vrc7exp);
		mainbus->registerdevice(vrc7exp);
		nsf_rom->vrc7exp = vrc7exp;
	}
	if (info.expansion_audio & NSF_EXP_MMC5) {
		mmc5exp = new mmc5audio();
		audbus->register_audible_device(mmc5exp);
		mainbus->registerdevice(mmc5exp);
		nsf_rom->mmc5exp = mmc5exp;
	}
	if (info.expansion_audio & NSF_EXP_FDS) {
		fdsexp = new fdsaudio();
		audbus->register_audible_device(fdsexp);
		mainbus->registerdevice(fdsexp);
		nsf_rom->fdsexp = fdsexp;
	}

	// tracknames.
	trackNames.clear();
	if (tracknames != nullptr) {
		std::string text;
		void* temp = tracknames;
		for (int i = 0; i < info.total_songs; i++) {
			temp = parse_data(text, (char*)temp);
			trackNames.push_back(text);
		}
		free(tracknames);
	}

	// track lengths
	trackLengths.clear();
	if (tracklengths != nullptr) {
		std::int32_t* l = (std::int32_t*)tracklengths;
		for (int i = 0; i < nr_lengths; i++) {
			trackLengths.push_back(l[i]);
		}
		free(tracklengths);
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
	songname = auth.title;
	artist = auth.artist;
	copyright = auth.copyright;
	ripper = auth.ripper;

	// disable bus conflicts.
	m_bus->emulate_bus_conflicts(true);

	nsf_mode = true;

	return true;
}

bool	cartridge::readstream_nsf(std::istream &nsffile, ppu *ppu_device, bus *mainbus, audio_player *audbus) {
	program = NULL;
	character = NULL;	// will not be used.

	nsf_header_raw nsf_hdr;
	nsffile.seekg(0, std::ios_base::beg);
	nsffile.read((char*)&nsf_hdr, sizeof(nsf_header_raw));

	// check for NSFe
	if (nsf_hdr.header_signature == (std::uint32_t)0x4546534E) {
		std::cout << "NSFe image detected.\n";
		return readstream_nsfe(nsffile, ppu_device, mainbus, audbus);
	}

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

	// compute NSF NMI trigger speed.
	// cpu cycles are 29780 per audio frame (60hz), lower that number and we increase refreshrate.
	// 16666 = 60.002hz that we know. 16666 = 29780 cycles.
	float cpu_cycles_per_frame = (float)(29780.0 / 16666.0) * (float)nsf_hdr.playspeed_ntsc;
	std::cout << "SPEED     : " << std::dec << (int)nsf_hdr.playspeed_ntsc;
	std::cout << " (" << (int)(1000000 / nsf_hdr.playspeed_ntsc) << " Hz, CPU cycles per audioframe: " << (int)cpu_cycles_per_frame << ")\n";

	// if >100hz enable high_hz_mode. OC'ing cpu 3x to keep up.
	if (((int)(1000000 / nsf_hdr.playspeed_ntsc)) > 100) {
		high_hz_nsf = true;
	} else high_hz_nsf = false;

	nsf_cpu_cycles = (int)cpu_cycles_per_frame;

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

	bool bankloading = false;
	for (int i = 0; i < 8; i++) if (nsf_hdr.bank_init[i] != 0x00) bankloading = true;

	int	prealloc = nsf_hdr.load_address - 0x8000;
	if (bankloading) prealloc &= 0x0FFF;

	// load program data.
	byte * program_data = (byte*)malloc(prealloc + program_size);

	if (program_data != nullptr)
		nsffile.read((char*)&program_data[prealloc], program_size);

	// NSF is loaded.
	// build a NSF cartridge.
	program = new nsfrom();
	nsfrom	*nsf_rom = reinterpret_cast<nsfrom*>(program);

	// configure NSF cartridge.
	nsf_rom->nmi_trig_cycles = (int)floorf(cpu_cycles_per_frame);
	nsf_rom->state.numsongs = nsf_hdr.total_songs;
	nsf_rom->state.currentsong = 1;
	for (int i = 0; i < 8; i++) nsf_rom->state.sbanks[i] = nsf_hdr.bank_init[i];
	nsf_rom->state.init = nsf_hdr.init_address;
	nsf_rom->state.play = nsf_hdr.play_address;
	nsf_rom->state.load = nsf_hdr.load_address;
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
	if (nsf_hdr.expansion_audio & NSF_EXP_NAMCO163) {
		namexp = new namco163audio();
		audbus->register_audible_device(namexp);
		mainbus->registerdevice(namexp);
		nsf_rom->namexp = namexp;
	}
	if (nsf_hdr.expansion_audio & NSF_EXP_VRC7) {
		vrc7exp = new vrc7audio();
		audbus->register_audible_device(vrc7exp);
		mainbus->registerdevice(vrc7exp);
		nsf_rom->vrc7exp = vrc7exp;
	}
	if (nsf_hdr.expansion_audio & NSF_EXP_MMC5) {
		mmc5exp = new mmc5audio();
		audbus->register_audible_device(mmc5exp);
		mainbus->registerdevice(mmc5exp);
		nsf_rom->mmc5exp = mmc5exp;
	}
	if (nsf_hdr.expansion_audio & NSF_EXP_FDS) {
		fdsexp = new fdsaudio();
		audbus->register_audible_device(fdsexp);
		mainbus->registerdevice(fdsexp);
		nsf_rom->fdsexp = fdsexp;
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
	songname = nsf_hdr.songname;
	artist = nsf_hdr.artist;
	copyright = nsf_hdr.copyright;
	ripper = "<?>";

	// disable bus conflicts.
	m_bus->emulate_bus_conflicts(true);

	nsf_mode = true;

	return true;
}

void	cartridge::readstream(std::istream &nesfile, ppu *ppu_device, bus *mainbus, audio_player *audbus, const char *orgfilename) {

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
		if (!readstream_nsf(nesfile, ppu_device, mainbus, audbus)) {
			std::cout << "Cartridge is invalid format\n";
		}
		return;
	}

	// load program data
	void * program_data = malloc(nes.programsize);
	void * char_data = malloc(16);

	if (!program_data || !char_data) {
		std::cout << "Failed to reserve memory for cartridge file." << std::endl;
		return;
	}

	nesfile.read((char*)program_data, nes.programsize);

	// look for the game in the db (if available)
	if (gamedb != nullptr) {
		if (gamedb->db_loaded) {
			// db is loaded.
			// find the game.
			int gameid = gamedb->in_db(program_data, nes.programsize);
			if (gameid != -1) {
				db_game gdb = gamedb->get_game_id(gameid);
				nes.has_nes20 = true;
				// display mapper mismatch with DB if any.
				if (nes.mapper != gdb.mapper) {
					std::cout << std::dec;
					std::cout << "Cartridge file has mismatching mapper, mapper: " << (int)nes.mapper << ", mapper in DB: " << (int)gdb.mapper << "\n";
				}
				nes.mapper = gdb.mapper;
				nes.submapper = gdb.submapper;
				nes.has_battery = gdb.has_battery > 0;
				nes.has_prg_ram = gdb.prgram_size > 0;
				nes.program_ram_size = gdb.prgram_size;
				nes.mirror_vertical = gdb.mirroring > 0;
			}
		}
	}

	std::cout << "cartridge is valid." << std::endl;
	std::cout << "Cartridge has NSF 2.0 header? " << (nes.has_nes20 ? "Yes" : "No") << "\n";
	std::cout << "Program size: " << std::dec << (int)nes.programsize << " bytes.." << std::endl;
	if (nes.charsize > 0) {
		std::cout << "Charrom size: " << (int)nes.charsize << " bytes.." << std::endl;
	}
	else {
		std::cout << "Cartridge contains VRAM" << std::endl;
	}

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
		if (has_char_data) {
			character->set_rom_data((byte *)char_data, nes.charsize);
		}
		else {
			reinterpret_cast<mmc1_vrom*>(character)->is_ram(true);
		}
		program->set_rom_data((byte *)program_data, nes.programsize);
		break;
	case 2:
		// UXROM
		program = new uxrom();
		if (has_char_data) {
			character = new vrom();
			character->set_rom_data((byte *)char_data, nes.charsize);
		}
		else {
			charram = new vram();
			character = charram;
		}
		program->set_rom_data((byte *)program_data, nes.programsize);
		break;
	case 3:
		// CNROM
		program = new cnrom();
		if (has_char_data) {
			character = new cnvrom();
			character->set_rom_data((byte *)char_data, nes.charsize);
			reinterpret_cast<cnrom*>(program)->link_vrom(reinterpret_cast<cnvrom*>(character));
		}
		else {
			charram = new vram();
			character = charram;	// should not happen.
		}
		program->set_rom_data((byte *)program_data, nes.programsize);
		break;
	case 4:
	case 44:
	case 52:
		// MMC3 / M52
		program = new mmc3_rom();
		character = new mmc3_vrom();
		// mmc3 linking.
		reinterpret_cast<mmc3_vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		reinterpret_cast<mmc3_rom*>(program)->link_vrom(reinterpret_cast<mmc3_vrom*>(character));
		if (has_char_data) {
			character->set_rom_data((byte *)char_data, nes.charsize);
		}
		else {
			// RAM based MMC3?
			charram = new vram();
			character = charram;			
		}
		program->set_rom_data((byte *)program_data, nes.programsize);
		if (nes.mapper == 52) reinterpret_cast<mmc3_rom*>(program)->set_mapper52_mode();
		if (nes.mapper == 44) reinterpret_cast<mmc3_rom*>(program)->set_mapper44_mode();
		break;
	case 7:
		// AXROM
		program = new axrom_rom();
		if (has_char_data) {
			character = new vrom();
			character->set_rom_data((byte*)char_data, nes.charsize);
		}
		else {
			charram = new vram();
			character = charram;
		}
		program->set_rom_data((byte *)program_data, nes.programsize);
		reinterpret_cast<axrom_rom*>(program)->link_ppu_ram(&ppu_device->vram);
		reinterpret_cast<axrom_rom*>(program)->update_banks();
		break;
	case 9:
	case 10:
		// MMC2
		program = new mmc2_rom();
		character = new mmc2_vrom();
		character->set_rom_data((byte*)char_data, nes.charsize);
		program->set_rom_data((byte *)program_data, nes.programsize);
		if (nes.mapper == 10)
			reinterpret_cast<mmc2_rom*>(program)->mmc4mode = true;
		reinterpret_cast<mmc2_vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		reinterpret_cast<mmc2_rom*>(program)->link_vrom(reinterpret_cast<mmc2_vrom*>(character));
		reinterpret_cast<mmc2_rom*>(program)->update_banks();
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
	case 85:
		// VRC7
		program = new vrc7rom();
		character = new vrc7vrom();
		vrc7exp = new vrc7audio();
		audbus->register_audible_device(vrc7exp);
		mainbus->registerdevice(vrc7exp);
		// vrc7 linking.
		reinterpret_cast<vrc7rom*>(program)->link_vrom(reinterpret_cast<vrc7vrom*>(character));
		reinterpret_cast<vrc7vrom*>(character)->link_ppu_bus(&ppu_device->vram);
		reinterpret_cast<vrc7rom*>(program)->audiochip = vrc7exp;
		if (has_char_data) {
			character->set_rom_data((byte*)char_data, nes.charsize);
		}
		else {
			reinterpret_cast<vrc7vrom*>(character)->switch_vram();
		}
		program->set_rom_data((byte*)program_data, nes.programsize);
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
	case 71:
		// Camerica
		program = new m71rom();
		charram = new vram();
		character = charram;
		program->set_rom_data((byte*)program_data, nes.programsize);
		break;
	case 73:
		// VRC3 Konami.
		program = new vrc3rom();
		charram = new vram();
		character = charram;
		program->set_rom_data((byte*)program_data, nes.programsize);
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
			bool vrc2a = false;
			// enable vrc mode for the follow combinations.
			if ((nes.mapper == 22) && (nes.submapper == 0)) {
				vrc2 = true;
				vrc2a = true;
			}
			if ((nes.mapper == 23) && (nes.submapper == 3)) vrc2 = true;
			if ((nes.mapper == 25) && (nes.submapper == 3)) vrc2 = true;
			reinterpret_cast<vrc2_4_rom*>(program)->link_vrom(reinterpret_cast<vrc2_4_vrom*>(character));
			reinterpret_cast<vrc2_4_rom*>(program)->vrc2_mode = vrc2;
			reinterpret_cast<vrc2_4_rom*>(program)->vrc2a_char_mode = vrc2a;
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
			int compat = 0;
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
	case 87:
		// J87 ROM
		program = new j87rom();
		if (has_char_data) {
			character = new j87vrom();
			character->set_rom_data((byte*)char_data, nes.charsize);
			reinterpret_cast<j87rom*>(program)->link_vrom(reinterpret_cast<j87vrom*>(character));
		}
		else {
			charram = new vram();
			character = charram;	// should not happen.
		}
		program->set_rom_data((byte*)program_data, nes.programsize);
		break;
	default:
		std::cout << "Mapper #" << std::dec << (int)nes.mapper << " is unknown to me" << std::endl;
		break;
	}

	if (program != nullptr)
		std::cout << "Mapper devicename: " << program->get_device_descriptor() << "\n";

	// load srm file.
	if ((orgfilename != nullptr) && (nes.has_battery) && (program != nullptr)) {
		std::string srmfile;
		srmfile += orgfilename;
		srmfile += ".srm";
		// we have a generated filename. assign it to the loaded program rom.
		strncpy(program->get_sram_filename(), srmfile.c_str(), SRAM_MAX_FILE_NAME);
		std::ifstream srm(srmfile.c_str(), std::ios::binary | std::ios::in);
		if (srm.good()) {
			// load the file.
			srm.seekg(0, std::ios::end);
			size_t srm_size = srm.tellg();
			srm.seekg(0, std::ios::beg);
			byte* sram = (byte*)malloc(srm_size);
			if (sram != NULL) srm.read((char*)sram, srm_size);
			// update cart.
			if (sram != NULL) program->set_battery_backed_ram(sram, srm_size);
			std::cout << "Battery packed SRAM loaded from: " << srmfile.c_str() << "\n";
		}
	}
	
	if (program != nullptr)
		program->battery = nes.has_battery;

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

cartridge::cartridge(std::istream &stream, ppu *ppu_device, bus *mainbus, audio_player *audbus, nesdb *db) {
	std::cout << "Loading cartridge from memory 0x" << std::hex << (std::uint64_t)&stream << std::endl;
	gamedb = db;
	readstream(stream, ppu_device, mainbus, audbus, nullptr);
}

cartridge::cartridge(const char *filename, ppu *ppu_device, bus *mainbus, audio_player *audbus, nesdb *db) {
	// check if file is FDS?
	// problems with FDS file that they may come with or without a 16 byte header.
	// we can test the validity by modulating with 65500 bytes and check if modulus is 0
	// do this with or without -16.
	std::size_t filesize = 0;

	// load & parse NES file.
	std::ifstream	nesfile;
	nesfile.open(filename, std::ios::binary | std::ios::in);
	nesfile.seekg(0, std::ios_base::end);
	filesize = nesfile.tellg();
	nesfile.seekg(0, std::ios_base::beg);
	
	bool is_fds = ((filesize % 65500) == 0) || (((filesize - 16) % 65500) == 0);

	if (is_fds) {
		std::cout << "Loading FDS image: " << filename << std::endl;
		if (!readstream_fds(nesfile, ppu_device, mainbus, audbus, filename)) {
			std::cout << "Invalid FDS image\n";
		}
	}
	else {
		std::cout << "Loading cartridge: " << filename << std::endl;
		gamedb = db;
		readstream(nesfile, ppu_device, mainbus, audbus, filename);
	}
	
	nesfile.close();
}

cartridge::~cartridge() {
	std::cout << "Cleaning up cartridge..\n";
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
		if (namexp) {
			m_aud->unregister_audible_device(namexp);
			m_bus->removedevice_select_base(namexp->devicestart);
			delete namexp;
		}
		if (vrc7exp) {
			m_aud->unregister_audible_device(vrc7exp);
			m_bus->removedevice_select_base(vrc7exp->devicestart);
			delete vrc7exp;
		}
		if (mmc5exp) {
			m_aud->unregister_audible_device(mmc5exp);
			m_bus->removedevice_select_base(mmc5exp->devicestart);
			delete mmc5exp;
		}
		if (fdsexp) {
			m_aud->unregister_audible_device(fdsexp);
			m_bus->removedevice_select_base(fdsexp->devicestart);
			delete fdsexp;
		}
	}
	std::cout << "Expanded audio devices cleaned up..\n";
	if (m_bus != NULL) {
		if (program != NULL) {
			m_bus->removedevice_select_base(program->devicestart);
			// has it a SRAM file assigned?
			char* srmfile = program->get_sram_filename();
			if (strlen(srmfile) > 0) {
				batterybackedram* ramtowrite = program->get_battery_backed_ram();
				if (ramtowrite->size > 0) {
					std::ofstream srm(srmfile, std::ios::binary | std::ios::out);
					std::cout << "Writing battery backed ram to: " << srmfile << "\n";
					if (ramtowrite != nullptr)
						if (ramtowrite->data != nullptr) {
							srm.write((char*)ramtowrite->data, ramtowrite->size);
						}
					srm.close();
				}
				free(ramtowrite);
			}
		}
	}
	if (l_ppu != NULL) {
		l_ppu->set_char_rom(NULL);
	}
	std::cout << "Memory bus (PPU) devices cleaned up..\n";
	if (program != NULL) delete program;
	if (character != NULL) delete character;
	std::cout << "ROMs cleaned up..\n";
}
