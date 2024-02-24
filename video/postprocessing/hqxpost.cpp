/*

		Wrapper code for the HQ?x library.
		Written by Peter Santing

		Expects source hqx to be in /3rdparty/hqx/

*/


#pragma warning(disable : 4996)

// build the postprocessor.

#include "hqxpost.h"

// this is not complicated.

hq2x::hq2x() {
	strncpy(getName(), "HQ2X - 2016 Bruno Ribeiro", MAX_POSTPROCESSOR_NAME);
}

hq3x::hq3x() {
	strncpy(getName(), "HQ3X - 2016 Bruno Ribeiro", MAX_POSTPROCESSOR_NAME);
}

void hq2x::process_image(std::uint32_t* pc_image, int width, int height) {
	if (renderedimage == nullptr) {
		// this filter doubles the resolution. so we need to allocate (width*2)*(height*2)*4 bytes.
		newWidth = width * 2;
		newHeight = height * 2;
		renderedimage = (uint32_t*)malloc(newWidth * newHeight * 4);	// allocate enough memory.
	}

	// process the image.
	upscaler.resize(pc_image, width, height, renderedimage);
}

void hq3x::process_image(std::uint32_t* pc_image, int width, int height) {
	if (renderedimage == nullptr) {
		// this filter triples the resolution. so we need to allocate (width*2)*(height*2)*4 bytes.
		newWidth = width * 3;
		newHeight = height * 3;
		renderedimage = (uint32_t*)malloc(newWidth * newHeight * 4);	// allocate enough memory.
	}

	// process the image.
	upscaler.resize(pc_image, width, height, renderedimage);
}
