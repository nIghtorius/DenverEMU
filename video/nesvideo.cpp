#include "nesvideo.h"
#include <malloc.h>
#include <cstdint>

nesvideo::nesvideo() {
	displaybuffer = (std::uint16_t *)malloc(122880); // buffer is 256x240 16 bits.
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

		std::uint16_t framepixel = ((ntscpalette[idx] >> 3) << 11) |
			((ntscpalette[idx + 1] >> 2) << 5) |
			((ntscpalette[idx + 2] >> 3));

		displaybuffer[x] = framepixel;
	}
}