/*

	Cartridge implementation of the Famicom Disk System.

*/

#include "fds.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

#pragma warning(disable : 4996)

fds_rom::fds_rom() {
	strncpy(get_device_descriptor(), "Denver FDS Hardware", MAX_DESCRIPTOR_LENGTH);
	// detect BIOS file and load it.
	std::ifstream fdsbios("disksys.rom", std::ios_base::binary);
	if (!fdsbios.good()) {
		std::cout << "FDS bios file not found. Please place it as disksys.rom in the working directory.\n";
		exit(128);
	}
	// allocate 32kB for PRGRAM (0x6000-0xDFFF)
	prgram = (byte*)malloc(32768);
	// allocate 8kB for the BIOS (0xE000-0xFFFF)
	biosrom = (byte*)malloc(8192);

	// load bios rom. it is just a binary dump of 8KB.. easily loaded.
	fdsbios.read((char*)biosrom, 8192);
	fdsbios.close();

	// setup memory map.
	devicestart = 0x4020;	// lesson learned: do not map to 0x4000 (it will unregister my CPU device :X )
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;

	set_debug_data();
}

fds_rom::~fds_rom() {
	if (prgram) free(prgram);
	if (biosrom) free(biosrom);
	// clean up disks.
	for (const diskentry* aDisk : disks) {
		if (aDisk->changemap) free(aDisk->changemap);
		if (aDisk->data) free(aDisk->data);
	}
}

/*
void fds_rom::bios_debug(const int addr) {
	if (addr == DBG_NINTENDO_HDR_CHECK) {
		std::cout << "BIOS is checking NINTENDO-HVC* header..\n";
	}
	if (addr == DBG_XFER_FAIL) {
		std::cout << "Transfer has failed. reason is in X-reg.\n";
		if (cpu) {
			std::cout << "X-register is: " << std::dec << (int)cpu->regs.x << "\n";
			byte spbu = cpu->regs.sp;
			std::cout << "Caller address is " << std::hex << (int)cpu->pullstack_word() << "\n";
			std::cout << " NextCaller address is " << std::hex << (int)cpu->pullstack_word() << "\n";
			std::cout << "State [scanningdisk] is " << state.scanningdisk << "\n";
			cpu->regs.sp = spbu;
		}
	}
}
*/

byte fds_rom::read(const int addr, const int addr_from_base, const bool onlyread) {
	// registers.
	// bios_debug(addr);
	byte status;
	switch (addr) {
	case FDS_DISK_STATUS_REG:
		status = 0x00;
		if (state.timer_irq_tripped) status |= 0x01;
		if (state.byte_transfer) status |= 0x02;
		//if (!state.crc_passed) status |= 0x10;
		if (state.end_head) status |= 0x40;
		status |= 0x80;
		if (!onlyread) {
			state.timer_irq_tripped = false;
			state.disk_irq_tripped = false;
			state.byte_transfer = false;
		}
		irq_enable = state.timer_irq_tripped || state.disk_irq_tripped;
		return status;
		break;
	case FDS_READ_DATA_REG:
		if (!onlyread) {
			state.byte_transfer = false;
			state.disk_irq_tripped = false;
		}
		irq_enable = state.timer_irq_tripped || state.disk_irq_tripped;
		return state.datareg;
		break;
	case FDS_DRIVE_STATUS_REG:
		if (!onlyread) state.disk_irq_tripped = false;
		irq_enable = state.timer_irq_tripped || state.disk_irq_tripped;
		status = 0x00;
		if (!disk_inserted) status |= 0x01;
		if (!state.scanningdisk || !disk_inserted) status |= 0x02;
		if (!disk_inserted) status |= 0x04;
		return status;
		break;
	case FDS_EXT_CONNECTOR_READ:
		return battery_good ? 0x80 : 0x00;	// battery OK.
		break;
	}

	if ((addr >= 0x6000) && (addr <= 0xDFFF)) return prgram[addr - 0x6000]; // Program RAM.
	return biosrom[addr - 0xE000]; // BIOS.
}

void fds_rom::write(const int addr, const int addr_from_base, const byte data) {
	// memory.
	if ((addr >= 0x6000) && (addr <= 0xDFFF)) {
		prgram[addr - 0x6000] = data;
		return;
	}
	// registers.
	switch (addr) {
	case FDS_TIMER_LOW_RELOAD:
		state.irq_load = (state.irq_load & 0xFF00) | data;
		break;
	case FDS_TIMER_HIGH_RELOAD:
		state.irq_load = (state.irq_load & 0x00FF) | (data << 8);
		break;
	case FDS_IRQ_CONTROL:
		if (!state.disk_enabled) break;
		state.irq_repeat = (data & 0x01) > 0;
		state.irq_enabled = (data & 0x02) > 0;
		if (state.irq_enabled) {
			state.irq_count = state.irq_load;
			//state.timer_irq_tripped = false;
		} else state.timer_irq_tripped = false;
		break;
	case FDS_MASTER_IO_ENABLE:
		state.disk_enabled = (data & 0x01) > 0;
		if (!state.disk_enabled) {
			state.timer_irq_tripped = false;
			state.irq_enabled = false;
			state.disk_irq_tripped = false;
			irq_enable = false;
		}
		state.audio_enabled = (data & 0x02) > 0;
		break;
	case FDS_WRITE_DATA_REG:
		state.byte_transfer = false;
		state.datareg_w = data;
		state.disk_irq_tripped = false;
		irq_enable = state.timer_irq_tripped || state.disk_irq_tripped;
		break;
	case FDS_CONTROL_REG:
		state.dont_stop_motor = (data & 0x01) > 0;
		state.do_not_scan_media = (data & 0x02) > 0;
		state.transfer_rw = (data & 0x04) == 0;
		state.mirror_hv = (data & 0x08) == 0;
		state.crc_transfer = (data & 0x10) > 0;		
		state.in_data = (data & 0x40) > 0;
		state.disk_io_irq_enabled = (data & 0x80) > 0;
		state.disk_irq_tripped = false;

		// ppu.
		if (ppudevice) {
			if (state.mirror_hv) {
				ppudevice->configure_vertical_mirror();
			}
			else {
				ppudevice->configure_horizontal_mirror();
			}
		}
		break;
	}
	// irq status.
	irq_enable = state.timer_irq_tripped || state.disk_irq_tripped;
	return;
}

void fds_rom::runirqtimers(int ticks) {
	// Timer IRQ
	if (!state.irq_enabled) return;

	if (state.irq_count < 0) {
		state.irq_count += state.irq_load;
		if (state.irq_repeat) {
			state.irq_enabled = true;
		}
		else {
			state.irq_enabled = false;
		}
		state.timer_irq_tripped = true;
		irq_enable = true;
	}
	else {
		state.irq_count -= ticks;
	}
}

void fds_rom::clockdiskinsertion() {
	if (state.insert_disk > 0) {
		state.insert_disk--;
		if (state.insert_disk == 0) disk_inserted = true;
	}
}

void fds_rom::updateCRC(const byte data) {
	for (uint16_t n = 0x01; n <= 0x80; n<<=1) {
		bool c = (state.crcaccumulator & 1);
		state.crcaccumulator >>= 1;
		if (c) state.crcaccumulator ^= 0x8408;
		if (data & n) state.crcaccumulator ^= 0x8000;
	}
}

byte fds_rom::readDisk() {
	return disk[state.disksector];
}

void fds_rom::writeDisk(const byte data) {
	byte curvalue = disk[state.disksector - 2];
	if (curvalue != data) {
		disk[state.disksector - 2] = data;
		// update changemap.
		patch[state.disksector - 2] = 1;
	}
}

int fds_rom::rundevice(int ticks) {
	runirqtimers(ticks);
	clockdiskinsertion();
	if (!state.dont_stop_motor || !disk_inserted) {
		state.scanningdisk = false;
		state.end_head = true;
		return ticks;
	}

	if (state.do_not_scan_media && (!state.scanningdisk)) {
		return ticks;
	}

	if (state.end_head) {
		state.cyclecount = 50000;
		state.end_head = false;
		state.disksector = 0;
		state.gapend = false;
	}

	if (state.cyclecount > 0) {
		state.cyclecount -= ticks;
	}
	else {
		byte shiftreg = 0x00;
		state.scanningdisk = true;
		bool do_disk_irq = state.disk_io_irq_enabled;
		if (!state.transfer_rw) {
			// reading.
			shiftreg = readDisk();
			if (!state.last_crc_transfer) updateCRC(shiftreg);
			if (!state.in_data) {
				state.gapend = false;
				state.crcaccumulator = 0x00;
			}
			else {
				if ((shiftreg != 0x00) && !state.gapend) {
					state.gapend = true;
					do_disk_irq = false;
				}
			}
			if (state.gapend) {
				state.byte_transfer = true;
				state.datareg = shiftreg;
				state.disk_irq_tripped = do_disk_irq;
				if (do_disk_irq) irq_enable = true;
			}
		}
		else {
			// writing.
			if (!state.crc_transfer) {
				state.byte_transfer = true;
				shiftreg = state.datareg_w;
				do_disk_irq = state.disk_io_irq_enabled;
				if (do_disk_irq) {
					state.disk_irq_tripped = do_disk_irq;
					irq_enable = true;
				}
			}
			if (!state.in_data) {
				shiftreg = 0x00;
			}
			if (!state.crc_transfer) {
				updateCRC(shiftreg);
			}
			else {
				if (!state.last_crc_transfer) {
					updateCRC (0x00);
					updateCRC (0x00);
				}
				shiftreg = state.crcaccumulator & 0xFF;
				state.crcaccumulator >>= 8;
			}

			writeDisk(shiftreg);
			state.gapend = false;
		}
		state.last_crc_transfer = state.crc_transfer;
		state.disksector++;
		if (state.disksector >= disksize) {
			state.dont_stop_motor = false;
		}
		else {
			state.cyclecount += CYCLES_PER_SECTOR;
		}
	}

	return ticks;
}

void fds_rom::add_disk(const byte* diskdata, const int size) {
	diskentry* myDisk = new diskentry();
	myDisk->data = (byte*)malloc(size);
	if (myDisk->data) memcpy((void*)&myDisk->data[0], (const void*)diskdata, size);
	myDisk->size = size;
	myDisk->changemap = (byte*)malloc(size);
	// clear changemap.
	if (!myDisk->changemap) {
		std::cout << "Unable to reserve memory for the changemap (FDS-disk). Halting application.\n";
		exit(-1);
		return;
	}
	if (myDisk->changemap) memset((void*)myDisk->changemap, 0x00, size);
	disks.push_back(myDisk);
}

void fds_rom::set_side(int side) {
	if (side < disks.size()) {
		disk_inserted = false;
		disk = disks[side]->data;
		disksize = disks[side]->size;
		patch = disks[side]->changemap;
		state.insert_disk = 900000;	// start counter for disk insertion.
	}
}

void fds_rom::set_debug_data() {
	debugger.add_debug_var("FDS", -1, NULL, t_beginblock);
	debugger.add_debug_var("IRQ Load Value", -1, &state.irq_load, t_word);
	debugger.add_debug_var("IRQ Count Value", -1, &state.irq_count, t_word);
	debugger.add_debug_var("IRQ Repeat", -1, &state.irq_repeat, t_bool);
	debugger.add_debug_var("IRQ Enabled", -1, &state.irq_enabled, t_bool);
	debugger.add_debug_var("Disk Enabled", -1, &state.disk_enabled, t_bool);
	debugger.add_debug_var("Audio Enabled", -1, &state.audio_enabled, t_bool);
	debugger.add_debug_var("Datareg read", -1, &state.datareg, t_byte);
	debugger.add_debug_var("Datareg write", -1, &state.datareg_w, t_byte);
	debugger.add_debug_var("Do not stop motor", -1, &state.dont_stop_motor, t_bool);
	debugger.add_debug_var("Do not scan media", -1, &state.do_not_scan_media, t_bool);
	debugger.add_debug_var("Scanning disk", -1, &state.scanningdisk, t_bool);
	debugger.add_debug_var("Transfer RW", -1, &state.transfer_rw, t_bool);
	debugger.add_debug_var("Mirror HV", -1, &state.mirror_hv, t_bool);
	debugger.add_debug_var("CRC Transfer", -1, &state.crc_transfer, t_bool);
	debugger.add_debug_var("Data IN", -1, &state.in_data, t_bool);
	debugger.add_debug_var("Disk IO IRQ Enabled", -1, &state.disk_io_irq_enabled, t_bool);
	debugger.add_debug_var("Amount of disksides", -1, &romsize, t_int);
	debugger.add_debug_var("Timer IRQ Tripped", -1, &state.timer_irq_tripped, t_bool);
	debugger.add_debug_var("Disk IRQ Tripped", -1, &state.disk_irq_tripped, t_bool);
	debugger.add_debug_var("Disk Transfer (byte)", -1, &state.byte_transfer, t_bool);
	debugger.add_debug_var("CRC passed", -1, &state.crc_passed, t_bool);
	debugger.add_debug_var("End of Head", -1, &state.end_head, t_bool);
	debugger.add_debug_var("Disk read/write-able", -1, &state.disk_rwable, t_bool);
	debugger.add_debug_var("Disk Cycle Count", -1, &state.cyclecount, t_int);
	debugger.add_debug_var("Current Disk Byte", -1, &state.disksector, t_int);
	debugger.add_debug_var("Disk inserted", -1, &disk_inserted, t_bool);
	debugger.add_debug_var("Battery OK", -1, &battery_good, t_bool);
	debugger.add_debug_var("Gap end", -1, &state.gapend, t_bool);
	debugger.add_debug_var("FDS", -1, NULL, t_endblock);
	
	debugger.add_debug_var("Vectors", -1, NULL, t_beginblock);
	debugger.add_debug_var("RESET", -1, &prgram[0xDFFC - 0x6000], t_addr);
	debugger.add_debug_var("IRQ", -1, &prgram[0xDFFE - 0x6000], t_addr);
	debugger.add_debug_var("NMI#1", -1, &prgram[0xDFF6 - 0x6000], t_addr);
	debugger.add_debug_var("NMI#2", -1, &prgram[0xDFF8 - 0x6000], t_addr);
	debugger.add_debug_var("NMI#3", -1, &prgram[0xDFFA - 0x6000], t_addr);
	debugger.add_debug_var("Vectors", -1, NULL, t_endblock);
}

void fds_rom::set_battery_backed_ram(byte* data, const std::size_t size) {
	// restore data.
	for (int i = 0; i < size;) {
		uint32_t sector;
		uint8_t diskId = data[i]; i++;
		sector = data[i]; i++;
		sector |= (uint32_t)(data[i]) << 8; i++;
		sector |= (uint32_t)(data[i]) << 16; i++;
		sector |= (uint32_t)(data[i]) << 24; i++;
		uint16_t count;
		count = data[i]; i++;
		count |= (uint16_t)(data[i]) << 8; i++;
		//std::cout << "Patching disk #" << std::dec << (int)diskId << ", sector: " << (int)sector << " to " << (int)sector + (int)count << "\n";
		for (int j = 0; j < count; j++) {
			disks[diskId]->changemap[sector] = 1;
			disks[diskId]->data[sector] = data[i];
			i++;
			sector++;
		}
	}
}

batterybackedram* fds_rom::get_battery_backed_ram() {
	// generate patch data (dwd)
	std::vector<byte> patchdata;
	patchdata.clear();
	int count = 0;

	for (int j = 0; j < disks.size(); j++) {
		bool patchrun = false;
		uint32_t sector = 0;
		uint16_t bytecount = 0;
		int index = 0;

		patch = disks[j]->changemap;
		disk = disks[j]->data;

		for (int i = 0; i < disks[j]->size; i++) {
			// write serialized.
			if (patch[i] && patchrun) {
				patchdata.push_back(disk[i]);
				count++;
				bytecount++;
			}
			if (patch[i] && !patchrun) {
				// data change.
				patchrun = true;
				sector = i;
				patchdata.push_back(j & 0xFF); // disk id.
				patchdata.push_back((sector >> 0) & 0xFF);
				patchdata.push_back((sector >> 8) & 0xFF);
				patchdata.push_back((sector >> 16) & 0xFF);
				patchdata.push_back((sector >> 24) & 0xFF);
				count += 5;
				index = count;
				patchdata.push_back(0x00);//dummy write.
				patchdata.push_back(0x00);//dummy write.
				count+=2;
				patchdata.push_back(disk[i]);
				count++;
				bytecount = 1;
			}
			if (!patch[i] && patchrun) {
				// done data change stream.
				patchdata[index] = (bytecount >> 0) & 0xFF;
				patchdata[index + 1] = (bytecount >> 8) & 0xFF;
				patchrun = false;
				//std::cout << "Patch block completed. ";
				//std::cout << std::dec << "sector start: " << (int)sector << ", sector end: " << (int)i << ", bytecount: " << (int)bytecount << "\n";
			}
		}
	}

	//std::cout << "Patch data is " << std::dec << (int)patchdata.size() << " bytes..\n";
	// data has been written.
	return new batterybackedram(&patchdata.begin()[0], (int)patchdata.size());
}
