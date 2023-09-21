/*
		This are the routines that simulate bus access of the NES gaming system.
		(c) 2018 - P. Santing aka nIghtorius
*/

#pragma once

#include <vector>

#define		MAX_DESCRIPTOR_LENGTH	128
#define		BUS_OPEN_BUS			0xF0

/*
	bus_device

	a bus device can be everything that resides on the cpu bus.
	for example: the cartridge, the APU, the PPU or an memory mapper.

	all devices have a start address and a end address
	the adresses can also be also masked (recommended to do so for example: mirroring)
*/

typedef	unsigned __int16 word;
typedef unsigned __int8  byte;

struct buslayout {
	char pins[16];
};

// basic device can have bus. not always.
class device {
private:
	char	*devicedescriptor;
public:
	int		tick_rate;	// sets the tick rate. for example: if ppu rate 1 then cpu rate is 3. ( for each 3 ppu ticks there is one cpu tick )
	bool	irq_enable;	// device IRQ
	bool	nmi_enable;
	int		ticker;
	int		tickstodo;
	int		ticksdone;
	bool	in_dma_mode;
	bool	dma_start;
	device();
	~device();
	virtual	int		rundevice(int ticks); // returns the amount of ticks it actually did.
	char *			get_device_descriptor();
	virtual	void	dma(byte *data, bool is_output, bool started);
};

// a device connected to a bus.
class bus_device: public device	{	// we base the bus device off the device class.
public:
	bool	processlayout;			// get set to true when specific pin configuration is set. false when not ( helps speeding up bus emulation )
	int		devicestart;
	int		deviceend;
	int		devicemask;
	buslayout	pinout;
	bus_device();
	virtual ~bus_device();
	word			compute_addr_from_layout(word addr);
	void			swappins(int pin1, int pin2);
	void			groundpin(int pin);
	void			vccpin(int pin);
	void			resetpins_to_default();
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual	byte	read(int addr, int addr_from_base);
};

class bus {
private:
	int		address;
	std::vector<bus_device *> devices;
	bool	no_bus_conflicts = false;		// default we do not emulate bus conflicts, it is costly.
public:	
	bus();
	~bus();
	// general reading/writing.
	void	writememory(int addr, byte data);
	byte	readmemory(int addr);
	word	readmemory_as_word(int addr);
	word	readmemory_as_word_wrap(int addr);
	void	write(byte data);
	byte	read();
	//	IRQ/NMI lines
	bool	nmi_pulled();
	bool	irq_pulled();
	// device registration
	void	registerdevice(bus_device *device);
	void	removedevice_select_base(int baseaddr);
	// reports (cout)
	void	reportdevices();
	void	emulate_bus_conflicts(bool enable);	// emulate busconflicts? (true is slower)
};