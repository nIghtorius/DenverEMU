#include "nesvideo.h"
#include <cstdlib>
#include <cstring>

#pragma warning(disable : 4996)

nesvideo::nesvideo() {
	displaybuffer = (std::uint32_t *)malloc(245760); // buffer is 256x240 32 bits.
}

nesvideo::~nesvideo() {
	free(displaybuffer); // free used ram.
}

Image* nesvideo::getFrame() {
	outImage.image = displaybuffer;
	outImage.size.width = 256;
	outImage.size.height = 240;
	return &outImage;
}

Image* nesvideo::getPostImage() {
	// processes all postprocessors.
	int w = 256;
	int h = 240;
	void* _image = displaybuffer;

	for (postprocessor* ip : postprocessors) {
		ip->process_image((uint32_t*)_image, w, h);
		Image* image = ip->getImage();
		// get what we need from it and dispose it.
		_image = image->image;
		w = image->size.width;
		h = image->size.height;
		free(image);
	}

	// return final postprocessedImage.
	outImage.image = _image;
	outImage.size.width = w;
	outImage.size.height = h;
	return &outImage;
}

// compute with emphasis.
static inline void c_w_e(std::uint8_t *r, std::uint8_t *g, std::uint8_t *b, const std::uint8_t e) {
	if (e & EMPHASIS_R) {
		*g -= *g < 0x2F ? 0x00 : 0x2F;
		*b -= *b < 0x2F ? 0x00 : 0x2F;
	}
	if (e & EMPHASIS_G) {
		*r -= *r < 0x2F ? 0x00 : 0x2F;
		*b -= *b < 0x2F ? 0x00 : 0x2F;
	}
	if (e & EMPHASIS_B) {
		*r -= *r < 0x2F ? 0x00 : 0x2F;
		*g -= *g < 0x2F ? 0x00 : 0x2F;
	}
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
		std::uint8_t emp = ppu_image[x] >> 8;
		std::uint8_t r = ntscpalette[idx + 2];
		std::uint8_t g = ntscpalette[idx + 1];
		std::uint8_t b = ntscpalette[idx];
		c_w_e(&b, &g, &r, emp);
		std::uint32_t framepixel = 0xFF000000 | r << 16 | g << 8 | b;
		displaybuffer[x] = framepixel;
	}
}

void nesvideo::add_overscan_borders() {
	for (int i = 0; i < 2048; i++) displaybuffer[i] = 0xFF000000;
	for (int i = 59392; i < 61440; i++) displaybuffer[i] = 0xFF000000;
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

Image* postprocessor::getImage() {
	return new Image((void*)renderedimage, imageSize(newWidth, newHeight));
}

imageSize postprocessor::getDimensions() {
	imageSize size(newWidth, newHeight);
	return size;
}
