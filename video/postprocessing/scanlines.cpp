/*

	Scanline postprocessor for Denver.
	Written by Peter Santing.

*/

#pragma warning(disable : 4996)

// build the postprocessor.

#include <cstring>
#include <cstdlib>
#include "scanlines.h"

scanlines::scanlines() {
	strncpy(getName(), "Denver - Scanline postprocessor", MAX_POSTPROCESSOR_NAME);
}

void scanlines::process_image(std::uint32_t* pc_image, int width, int height) {
	if (renderedimage == nullptr) {
		// initialize.
		newWidth = width;
		newHeight = height * 2;
		renderedimage = (uint32_t*)malloc(newWidth * newHeight * 4); // allocate memory.
		if (renderedimage == nullptr) return;
	}

	// process the image.
	// what it does. 1st scanline (image), 2nd scanline (none, or 50, 70, 80 or 90%), 3rd scanline (image), etc.

	for (int y = 0; y < height; y++) {
		std::uint32_t* scanline = &pc_image[y * width];
		std::uint32_t* sc1 = &renderedimage[(y << 1) * newWidth];
		std::uint32_t* sc2 = &renderedimage[((y << 1) + 1) * newWidth];
		for (int x = 0; x < width; x++) {
			sc1[x] = scanline[x];	// just copy.
			switch (scanlinemode) {
			case scanlinetypes::h50:
				sc2[x] = 0xFF000000;
				break;
			case scanlinetypes::h75:
				sc2[x] = (scanline[x] & 0x00FEFEFE) >> 1;
				sc2[x] |= 0xFF000000;
				break;
			case scanlinetypes::h85:
				sc2[x] = (scanline[x] & 0x00FEFEFE) >> 1;
				sc2[x] += (scanline[x] & 0x00FEFEFE);
				sc2[x] &= 0x00FEFEFE;
				sc2[x] >>= 1;
				sc2[x] |= 0xFF000000;
				break;
			case scanlinetypes::h90:
				sc2[x] = (scanline[x] & 0x00FEFEFE) >> 1;
				sc2[x] += (scanline[x] & 0x00FEFEFE);
				sc2[x] &= 0x00FEFEFE;
				sc2[x] >>= 1;
				sc2[x] += (scanline[x] & 0x00FEFEFE);
				sc2[x] &= 0x00FEFEFE;
				sc2[x] >>= 1;
				sc2[x] |= 0xFF000000;
				break;
			}
		}
	}
}
