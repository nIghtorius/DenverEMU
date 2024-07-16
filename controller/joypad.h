/*

	joypad.h

	Handles controller events.

*/

#pragma once

#include <SDL.h>
#include <cstdint>
#include "../bus/bus.h"
#include <vector>

#define MAX_CONTROLLERS			4
#define CTR_STROBE_PORT			0x16
#define CTR_CTRL1_PORT			0x16
#define CTR_CTRL2_PORT			0x17

#define CTR_BIT_STROBE			0x01
#define CTR_READ_CTRL			0x01

#define DEADZONE				16384

struct controller {
	bool	states[8];
	bool	right, left, down, up;
	bool	start, select;
	bool	b, a;
}; // state of controller.

struct controller_cfg {
	int		left, up, right, down, a, b, select, start;
}; // codes for which button is what.

class joypad {
private:
	controller_cfg	cfg[MAX_CONTROLLERS];
	controller_cfg  cfg_gc[MAX_CONTROLLERS];
	controller	controllers[MAX_CONTROLLERS];
	uint8_t		readouts[MAX_CONTROLLERS];
	void	detect_controllers();
	void	reset_controller_detect();
public:
	std::vector<SDL_GameController*> gameControllers;	// publicly available.
	int		controllermapping[MAX_CONTROLLERS];
	joypad();
	~joypad();
	void	set_default_configs();
	void	process_kb_event(SDL_Event *event);
	void	process_controller_event(SDL_Event *event);
	void	process_controller_connect_event(SDL_Event* event);
	bool	pulse_read_out(int controller_id);
	void	strobe(int controller_id);
};

class nes_2a03_joyports : public bus_device {
private:
public:
	nes_2a03_joyports();
	nes_2a03_joyports(joypad * ctrl);
	~nes_2a03_joyports();

	joypad * controller;

	void write(int addr, int addr_from_base, byte data);
	byte read(int addr, int addr_from_base, bool onlyread = false);
};