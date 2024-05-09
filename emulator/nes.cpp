/*

	Implementation of the true emulator.
	(c) 2023 P. Santing.

*/

#include "nes.h"
#include "denverlogo.h"

struct imembuf : std::streambuf
{
	imembuf(const char* base, size_t size)
	{
		char* p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};

struct imemstream : virtual imembuf, std::istream
{
	imemstream(const char* mem, size_t size) :
		imembuf(mem, size),
		std::istream(static_cast<std::streambuf*>(this))
	{
	}
};

nes_emulator::nes_emulator() {
	// setup the emulator.
	mainbus = new bus();
	nes_2a03 = new package_2a03();
	nesram = new mainram();
	ppu_device = new ppu();
	audio = new audio_player();
	joydefs = new joypad();
	nes_2a03->set_joydefs(joydefs);
	cart = nullptr;

	// configure links.
	clock.setdevices(nes_2a03, ppu_device, nullptr);
	mainbus->registerdevice(nes_2a03);
	mainbus->registerdevice(nesram);
	mainbus->registerdevice(ppu_device);
	mainbus->emulate_bus_conflicts(true);

	// configure audio.
	audio->boostspeed = false;
	audio->register_audible_device(&nes_2a03->apu_2a03);

	// configure video.
	video_out = new nesvideo();
	
	// start audio.
	audio->startplayback();
}

std::vector<postprocessor*> nes_emulator::listOfPostProcessors() {
}

nes_emulator::~nes_emulator() {
	// destroy the emulator.
	audio->stopplayback();
	if (cart) delete cart;
	delete audio;
	delete mainbus;	// also kills the linked devices.
	delete joydefs;
	delete video_out;
}

void	nes_emulator::reset() {
	nes_2a03->cpu_2a03.reset();
}

void	nes_emulator::cold_reset() {
	nes_2a03->cpu_2a03.coldboot();
}

void	nes_emulator::run_till_frame_ready(void (*callback)(SDL_Event*)) {
	while (!ppu_device->isFrameReady()) {		
		clock.step();
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (callback) callback(&event);
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if (event.key.keysym.scancode == 0x35) {
					audio->boostspeed = (event.type == SDL_KEYDOWN);
				}
				joydefs->process_kb_event(&event);
				break;
			}
		}
	}
	// frame is ready.. generate frame.
	if (ppu_device) video_out->process_ppu_image((std::uint16_t *)ppu_device->getFrameBuffer());
}

void	nes_emulator::fast_run_callback() {
	clock.run();
}

void	nes_emulator::prepare_frame() {
	if (ppu_device) {
		video_out->process_ppu_image((std::uint16_t *)ppu_device->getFrameBuffer());
		video_out->add_overscan_borders();
	}
}

void	nes_emulator::sync_audio() {
	audio->play_audio();	// blocking call, releases when a frame of audio has been played (16.66ms NTSC)
}

nes_frame_tex * nes_emulator::returnFrameAsTexture() {
	frame.format = GL_RGBA;

	Image* myImage = video_out->getPostImage();

	frame.w = myImage->size.width;
	frame.h = myImage->size.height;
	frame.texture = myImage->image;
	return &frame;
}

bool	nes_emulator::hasquit() {
	return quit;
}

void	nes_emulator::stop() {
	quit = true;
	clock.running = false;
}

void	nes_emulator::load_cartridge(const char * filename) {
	if (cart) delete cart;
	cart = new cartridge(filename, ppu_device, mainbus, audio);
	clock.setdevices(nes_2a03, ppu_device, cart->program);
	nes_2a03->cpu_2a03.coldboot();
}

void	nes_emulator::load_logo() {
	if (cart) delete cart;
	imemstream data((char *)denverlogo, sizeof(denverlogo));
	cart = new cartridge(data, ppu_device, mainbus, audio);
	clock.setdevices(nes_2a03, ppu_device, cart->program);
	nes_2a03->cpu_2a03.coldboot();
}

void	nes_emulator::renderFrameToGL(const int windowWidth, const int windowHeight, const GLuint tex) {
	int wH, wW;
	int fW = windowWidth;
	int fH = windowHeight;

	float scale_x = fW / 256.0f;
	float scale_y = fH / 240.0f;

	float width_x = 0.0f;
	float height_y = 0.0f;

	if (scale_x > scale_y) {
		// scale y dominant.
		width_x = 256.0f * scale_y;
		height_y = 240.0f * scale_y;
	}
	else {
		// scale x dominant.
		width_x = 256.0f * scale_x;
		height_y = 240.0f * scale_x;
	}

	wW = (int)floor(width_x);
	wH = (int)floor(height_y);
	scale_x *= 1.3f;
	int start_x = (int)floor((fW / 2) - (width_x / 2));

	glViewport(0, 0, fW, fH);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(start_x, 0, wW, wH);

	if (shader) {
		glUseProgram(shader);

		// these parameters are required in your pixelshader in order to properly show the screen.
		GLint screen = glGetUniformLocation(shader, "screen");
		GLint nesvideo = glGetUniformLocation(shader, "nesvideo");
		GLint offset = glGetUniformLocation(shader, "offset");
		glUniform3f(screen, width_x, height_y, 1.0f);		// pass screen size to shader. (actual pixels)
		glUniform1i(nesvideo, 0);							// tell which texture engine has the texture. (TEXTURE1)
		glUniform2f(offset, (float)start_x, 0);					// pass offset to shader. (how much the render is too the right)
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 256.0f, 0.0f, 240.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex3f(0, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(0, 240, 0);
	glTexCoord2f(1, 0); glVertex3f(256, 240, 0);
	glTexCoord2f(1, 1); glVertex3f(256, 0, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


void	nes_emulator::use_shader(const char* filename) {
	// remove previous shader.
	glDeleteProgram(shader);
	// load and compiles shader and applies it to the rendering pipeline.
	GLuint fragShader;
	std::cout << "Compiling shader: " << filename << "....\n";
	// load file.
	std::ifstream shaderFile (filename, std::ios::in | std::ios::binary);
	if (!shaderFile.good()) {
		std::cout << "Shader not found, aborting.\n";
		return;
	}
	shaderFile.seekg(0, std::ios_base::end);
	std::size_t shaderSize = shaderFile.tellg();
	shaderFile.seekg(0, std::ios_base::beg);
	GLchar* shaderSource = (GLchar*)malloc(shaderSize + 1);
	if (shaderSource == nullptr) {
		return;
	}
	memset(shaderSource, 0, shaderSize + 1);
	shaderFile.read(shaderSource, shaderSize);
	shaderFile.close();
	
	// compile.
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &shaderSource, NULL);
	glCompileShader(fragShader);

	GLint isCompiled = 0;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		if (maxLength > 2) {
			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(fragShader);

			std::cout << "Unable to load shader: " << filename << "\n";
			std::cout << "Reason:\n" << infoLog.data() << "\n";
		}
		else {
			std::cout << "Unable to load shader, reason unknown..\n";
		}
		return;
	}
	std::cout << "Shader compiled succesfully..\nShader ID: " << (int)fragShader << "\n";
	free(shaderSource);
	// make a program.
	shader = glCreateProgram();
	glAttachShader(shader, fragShader);
	glLinkProgram(shader);
	glDeleteShader(fragShader);
	std::cout << "Shaderprogram linked succesfully..\nShaderProg ID: " << (int)shader << "\n";
}
