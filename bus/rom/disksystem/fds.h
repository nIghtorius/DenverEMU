/*

	FDS Cartridge part
	(c) 2024 P. Santing

	Emulates FDS cart hardware.

*/

#pragma once

#include "../rom.h"
#include "../../video/ppu.h"
#include "../../cpu/cpu2a03_fast.h"
#include <vector>

// write registers.
#define		FDS_TIMER_LOW_RELOAD 0x4020
#define		FDS_TIMER_HIGH_RELOAD 0x4021
#define		FDS_IRQ_CONTROL 0x4022
#define		FDS_MASTER_IO_ENABLE 0x4023
#define		FDS_WRITE_DATA_REG 0x4024
#define		FDS_CONTROL_REG 0x4025
#define		FDS_EXT_CONNECTOR_WRITE 0x4026

// read registers.
#define		FDS_DISK_STATUS_REG 0x4030
#define		FDS_READ_DATA_REG 0x4031
#define		FDS_DRIVE_STATUS_REG 0x4032
#define		FDS_EXT_CONNECTOR_READ 0x4033
#define		CYCLES_PER_SECTOR 150

// BIOS
#define		FDS_BIOS_CHECKDISKHEADER 0xE445
#define		DBG_NINTENDO_HDR_CHECK 0xE6F5		// ldy #0x0d
#define		DBG_XFER_FAIL_ON_NEQ	0xE77F
#define		DBG_XFER_FAIL_ON_CY		0xE77C
#define		DBG_XFER_FAIL			0xE781


struct fds_state {
	word irq_load = 0;
	int  irq_count = 0;
	bool irq_repeat = false;
	bool irq_enabled = false;
	bool disk_enabled = false;
	bool audio_enabled = false;
	byte datareg = 0x00;
	byte datareg_w = 0x00;
	bool dont_stop_motor = false;
	bool transfer_rw = false;
	bool mirror_hv = false;
	bool crc_transfer = false;
	bool last_crc_transfer = false;
	bool in_data = false;
	bool disk_io_irq_enabled = false;
	bool byte_transfer = false;
	bool timer_irq_tripped = false;
	bool disk_irq_tripped = false;
	bool crc_passed = false;
	bool end_head = false;
	bool disk_rwable = false;
	bool do_not_scan_media = false;

	int disksector = 0x00;
	int cyclecount = 0x00;

	bool gapend = false;
	bool scanningdisk = false;

	uint16_t crcaccumulator = 0x00;
	int	insert_disk = 0;
};

class fds_rom : public rom {
private:
	struct diskentry {
		byte* data;
		int	size;
		byte* changemap;
	};

	byte* prgram = nullptr;
	byte* biosrom = nullptr;
	byte* disk = nullptr;
	byte* patch = nullptr;

	int	  disksize = 0;

	void runirqtimers(int ticks);
	void clockdiskinsertion();
	void updateCRC(const byte data);

	byte readDisk();
	void writeDisk(const byte data);

	//void bios_debug(const int addr);

public:
	fds_rom();
	~fds_rom();

	fds_state state;
	ppu* ppudevice = nullptr;

	//cpu2a03_fast *cpu;

	std::vector<diskentry*> disks;

	bool	disk_inserted = true;
	bool	battery_good = true;

	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void write(const int addr, const int addr_from_base, const byte data);
	virtual int rundevice(int ticks);

	void	add_disk(const byte* diskdata, const int size);
	void	set_side(int side);
	void	set_debug_data();

	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, const std::size_t size);
};
