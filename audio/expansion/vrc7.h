/*

	Konami VRC7 Sound Chip
	(c) 2024 Multiple Authors.
	Denver Module written by P. Santing

*/

#pragma once

#include "../../bus/bus.h"
#include "../audio.h"
#include <vector>

#include "../../3rdparty/fm/emu2413.h"		// tricky.. outputs int16 data. Audio lib expects floats.

class vrc7audio : public audio_device {
private:
	OPLL* opl = nullptr;
	byte		regselect = 0;
	float		normalize = 0.0f;
	float		sample = 0.0f;
	int			samplehold = 0;
	int			msamplehold = 0;

public:
	vrc7audio();
	~vrc7audio();
	void	reset();
	int		rundevice(int ticks);
	void	write(int addr, int addr_from_base, byte data);
	virtual void	set_debug_data();
};
