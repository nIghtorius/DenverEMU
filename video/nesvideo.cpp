#include "nesvideo.h"
#include <cstdlib>

nesvideo::nesvideo() {
	displaybuffer = (std::uint32_t *)malloc(245760); // buffer is 256x240 32 bits.
	displaybufferx = (std::uint32_t *)malloc(1024 * 960 * 4); 
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

void nesvideo::add_overscan_borders() {
	for (int i = 0; i < 2048; i++) displaybuffer[i] = 0;
	for (int i = 59392; i < 61440; i++) displaybuffer[i] = 0;
}

void nesvideo::hq2x_image() {
	hq2x.resize(displaybuffer, 256, 240, displaybufferx);
}

void nesvideo::hq3x_image() {
	hq3x.resize(displaybuffer, 256, 240, displaybufferx);
}

void * nesvideo::getFramex() {
	return displaybufferx;
}