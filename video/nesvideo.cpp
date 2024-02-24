#include "nesvideo.h"
#include <cstdlib>
#pragma warning(disable : 4996)

nesvideo::nesvideo() {
	displaybuffer = (std::uint32_t *)malloc(245760); // buffer is 256x240 32 bits.
}

nesvideo::~nesvideo() {
	free(displaybuffer); // free used ram.
}

void * nesvideo::getFrame() {
	return displaybuffer;
}

postprocessedImage* nesvideo::getPostImage() {
	// processes all postprocessors.
	int w = 256;
	int h = 240;
	void* _image = displaybuffer;

	for (postprocessor* ip : postprocessors) {
		ip->process_image((uint32_t*)_image, w, h);
		postprocessedImage* image = ip->getImage();
		// get what we need from it and dispose it.
		_image = image->image;
		w = image->size.width;
		h = image->size.height;
		free(image);
	}

	// return final postprocessedImage.
	return new postprocessedImage(_image, imageSize(w, h));
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

void nesvideo::RegisterPostProcessor(postprocessor* processor) {
	postprocessors.push_back(processor);
}

void nesvideo::RemovePostProcessor(postprocessor* processor) {
	for (int i = 0; i < postprocessors.size(); i++) {
		if (postprocessors[i] == processor) {
			// remove the processor.
			postprocessors.erase(postprocessors.begin() + i);
			return;
		}
	}
}

void nesvideo::ClearPostProcessors() {
	postprocessors.clear();
}

// postprocessor framework.
postprocessor::postprocessor() {
	name = (char*)malloc(MAX_POSTPROCESSOR_NAME + 1);
	strncpy(getName(), "Denver postprocesser - null", MAX_POSTPROCESSOR_NAME);
}

postprocessor::~postprocessor() {
	if (renderedimage != nullptr) free(renderedimage);
	free(name);
}

char * postprocessor::getName() {
	return name;
}

void postprocessor::process_image(std::uint32_t* pc_image, int width, int height) {
	
}

postprocessedImage* postprocessor::getImage() {
	return new postprocessedImage((void*)renderedimage, imageSize(newWidth, newHeight));
}

imageSize postprocessor::getDimensions() {
	imageSize size(newWidth, newHeight);
	return size;
}
