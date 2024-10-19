/*
		This are the routines that simulate bus access of the NES gaming system.
		(c) 2018 - P. Santing aka nIghtorius
*/

#pragma once

#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include "../debug/device_debugger.h"

#define		MAX_DESCRIPTOR_LENGTH	128
#define		BUS_OPEN_BUS			0xF0

/*
	bus_device

	a bus device can be everything that resides on the cpu bus.
	for example: the cartridge, the APU, the PPU or an memory mapper.

	all devices have a start address and a end address
	the adresses can also be also masked (recommended to do so for example: mirroring)
*/

typedef	std::uint16_t word;
typedef std::uint8_t  byte;

struct buslayout {
	char pins[16];
};

// basic device can have bus. not always.
class device {
private:
	char	*devicedescriptor;
public:
	int		tick_rate;	// sets the tick rate. for example: if ppu rate 1 then cpu rate is 3. ( for each 3 ppu ticks there is one cpu tick )
	bool	irq_enable = false;	// device IRQ
	bool	nmi_enable = false;
	int		ticker = 0;
	int		tickstodo = 0;
	int		ticksdone = 0;
	bool	in_dma_mode = false;
	bool	dma_start = false;
	device();
	~device();
	virtual	int		rundevice(const int ticks); // returns the amount of ticks it actually did.
	char *			get_device_descriptor();
	virtual	void	dma(byte *data, bool is_output, bool started);
	virtual void	reset();
};

class bus;

// a device connected to a bus.
class bus_device: public device	{	// we base the bus device off the device class.
public:
	bool	processlayout = false;			// get set to true when specific pin configuration is set. false when not ( helps speeding up bus emulation )
	bool	trigger_a_change = false;
	int		devicestart;
	int		deviceend;
	int		devicemask;
	bus*	devicebus = nullptr;
	device_debugger debugger;
	buslayout	pinout;
	bus_device();
	virtual ~bus_device();
	word			compute_addr_from_layout(const word addr);
	void			swappins(const int pin1, const int pin2);
	void			groundpin(const int pin);
	void			vccpin(const int pin);
	void			resetpins_to_default();
	void			set_adres_intent(word address);
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual	byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void	_attach_to_bus(bus * attachedbus);
	virtual void	set_debug_data();
	virtual void	a_line_change(const word newaddress);
};

class bus {
private:
	bool	no_bus_conflicts = false;		// default we do not emulate bus conflicts, it is costly.
public:	
	bus();
	~bus();
	int		address = 0;
	// general reading/writing.
	std::vector<bus_device *> devices;
	void	writememory(const int addr, const byte data);
	byte	readmemory(const int addr, const bool onlyread = false);
	word	readmemory_as_word(const int addr, const bool onlyread = false);
	word	readmemory_as_word_wrap(const int addr, const bool onlyread = false);
	void	write(const byte data);
	byte	read(const bool onlyread = false);
	//	IRQ/NMI lines
	bool	nmi_pulled();
	bool	irq_pulled();
	// device registration
	void	registerdevice(bus_device *device);
	void	removedevice_select_base(const int baseaddr);
	// reports (cout)
	void	reportdevices();
	void	emulate_bus_conflicts(const bool enable);	// emulate busconflicts? (true is slower)
	void	busreset();
	// finding devices.
	bus_device* find_device_partial_name_match(const std::string matchstring);
	// selecting address.
	void	trigger_address_update(int _address);
};
