#pragma once

#include "../nesvideo.h"
#include "../3rdparty/hqx/HQ2x.h"
#include "../3rdparty/hqx/HQ3x.h"

/*

		scanline postprocessor.
		(c) 2024 Peter Santing

*/

enum scanlinetypes {
	h50, h75, h85, h90
};

class scanlines : public postprocessor {
private:
public:
	scanlinetypes	scanlinemode = scanlinetypes::h50;
	scanlines();
	void process_image(std::uint32_t* pc_image, int width, int height);
};
