#include "nesvideo.h"
#include <cstdlib>
#include <cstdint>

nesvideo::nesvideo() {
	displaybuffer = (std::uint32_t *)malloc(245760); // buffer is 256x240 16 bits.
}

nesvideo::~nesvideo() {
	free(displaybuffer); // free used ram.
}

void * nesvideo::getFrame() {
	return displaybuffer;
}

void nesvideo::process_ppu_image(std::uint16_t * ppu_image) {
	/*
		input is a buffer of 16 bits per pixel.
		pixel data is made up as
		[xxxxx]e[rgb][pppppppp]
		[rgb] is emphasis bits, [pppppppp] is pixel data. [xxxxx] is filler data we ignore (no need)
	*/

	for (int x = 0; x < 61440; x++) {
		std::uint8_t pixel = ppu_image[x] & 0x3F;
		
		std::int32_t idx = pixel * 3;

		std::uint32_t framepixel = 0xFF000000 | ((ntscpalette[idx+2]) << 16) |
			((ntscpalette[idx + 1]) << 8) |
			((ntscpalette[idx]));

		displaybuffer[x] = framepixel;
	}
}