#pragma once

#include "../nesvideo.h"
#include "../3rdparty/hqx/HQ2x.h"
#include "../3rdparty/hqx/HQ3x.h"

/*

		HQ2x/HQ3x postprocessor wrapper.
		(c) 2024 Peter Santing

*/


class hq2x : public postprocessor {
private:
	HQ2x upscaler;
public:
	hq2x();
	void process_image(std::uint32_t* pc_image, int width, int height);
};

class hq3x : public postprocessor {
private:
	HQ3x upscaler;
public:
	hq3x();
	void process_image(std::uint32_t* pc_image, int width, int height);
};

