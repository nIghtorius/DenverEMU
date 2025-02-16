/*

	Implementation of the true emulator.
	(c) 2023 P. Santing.

*/

#include "nes.h"
#include "denverlogo.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "../denvergui/dengui_main.h"
#include "../denvergui/gui_debuggers.h"

#include "../video/debug_renderer.h"

#include <filesystem>

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

	time = SDL_GetTicks64();		// get time.
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

bool	nes_emulator::hasquit() const {
	return quit;
}

void	nes_emulator::stop() {
	quit = true;
	clock.running = false;
}

void	nes_emulator::load_cartridge(const char * filename) {
	if (cart) delete cart;
	cart = new cartridge(filename, ppu_device, mainbus, audio, db);
	clock.setdevices(nes_2a03, ppu_device, cart->program);
	// check for NSF.
	clock.nsf_mode = cart->high_hz_nsf;
	nes_2a03->nsf_mode = cart->high_hz_nsf;
	if (cart->high_hz_nsf) {
		// set sync block from 1 unit to (nsf_cpu_cycles / 4) * 3
		clock.set_sync_cycle_in_ppucycles((cart->nsf_cpu_cycles >> 2) * 3);
	}
	else clock.set_sync_cycle_in_ppucycles(1);
	nes_2a03->cpu_2a03.coldboot();
	audio->update_devices();
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

	// compute iTime.
	uint64_t ms_elapsed = SDL_GetTicks64() - time;
	float iTime = (float)ms_elapsed / 1000;

	glViewport(0, 0, fW, fH);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(start_x, 0, wW, wH);

	if (shader) {
		glUseProgram(shader);

		// these parameters are required in your pixelshader in order to properly show the screen.
		GLint screen = glGetUniformLocation(shader, "screen");
		GLint nesvideo = glGetUniformLocation(shader, "nesvideo");
		GLint offset = glGetUniformLocation(shader, "offset");
		GLint itime = glGetUniformLocation(shader, "iTime");
		glUniform3f(screen, width_x, height_y, 1.0f);		// pass screen size to shader. (actual pixels)
		glUniform1i(nesvideo, 0);							// tell which texture engine has the texture. (TEXTURE1)
		glUniform2f(offset, (float)start_x, 0);					// pass offset to shader. (how much the render is too the right)
		glUniform1f(itime, iTime);
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
	glTexCoord2f(1, 0); glVertex3f(255, 240, 0);
	glTexCoord2f(1, 1); glVertex3f(255, 0, 0);
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

extern std::vector<std::string> shaderList;
extern bool show_controllers;
extern bool fullscreen;
extern int p1_controller;
extern int p2_controller;
extern bool vsync_enable;
extern float override_scale;
extern int width;
extern int height;
extern bool linear_filter;
extern bool no_audio;
extern bool no_audio_emu;
extern bool no_expanded_audio;
extern char	rom_load_startup[512];
extern char	shader_load_startup[512];
extern bool no_gui;

void nes_emulator::start() {
	// Load DB
	nesdb nes20db;

	// SDL
	std::cout << "Setting up SDL.." << std::endl;

	// Collect shaders for the GUI.
	shaderList.clear();
	std::string path = "./shaders";
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		shaderList.push_back(entry.path().string());
	}

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char* glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	int flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
	if (fullscreen) {
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
	}

	db = &nes20db;
	if (show_controllers) {
		// we might have initialized denver, but the init will stop there. But we will show the detected controllers.
		std::cout << "Denver has detected the following gamecontrollers.\n\n";
		for (int i = 0; i < joydefs->gameControllers.size(); i++) {
			const char* controllername = SDL_GameControllerName(joydefs->gameControllers[i]);
			std::cout << std::dec << i << ". " << controllername << "\n";
		}
		std::cout << "\n\nDenver will exit now...\n";
		exit(0);
	}

	// configure controller mapping (commandline)
	joydefs->controllermapping[0] = p1_controller;
	joydefs->controllermapping[1] = p2_controller;

	SDL_Window* win = SDL_CreateWindow("Denver NES emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

	SDL_GLContext gl_context = SDL_GL_CreateContext(win);
	SDL_GL_MakeCurrent(win, gl_context);
	int vsync = vsync_enable ? 1 : 0;
	if (SDL_GL_SetSwapInterval(vsync)) {
		std::cout << "Unable to set Vsync" << std::endl;
	}

	GLuint tex;
	GLuint ppu_ptable;
	GLuint ppu_ntable;

	// Texture.
	glGenTextures(1, &tex);
	glGenTextures(1, &ppu_ptable);
	glGenTextures(1, &ppu_ntable);

	glBindTexture(GL_TEXTURE_2D, tex);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	float ddpi = 0.0f;
	SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
	float display_scale = ddpi / (float)96.0f;
	display_scale = display_scale < (float)1.0f ? (float)1.0 : display_scale;

	if (override_scale > 0.0f) display_scale = override_scale;

	std::cout << "Window Scale will be " << display_scale << "x\n";

	// Setup Dear ImGui style
	ImFontConfig* fConfig = new ImFontConfig();
	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(display_scale);
	io.Fonts->AddFontFromFileTTF(
		"fonts/denver-ui.ttf", 16 * display_scale, fConfig, io.Fonts->GetGlyphRangesDefault()
	)->Scale = 1.0;

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(win, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);
	glewInit();

	int time = SDL_GetTicks();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool btrue = true;
	bool* p_open = &btrue;

	
	if (strnlen(shader_load_startup, 512) != 0) {
		use_shader(shader_load_startup);
	}

	if (no_audio) audio->no_audio = true;
	if (no_audio_emu) nes_2a03->no_apu = true;
	if (no_expanded_audio) audio->no_expanded_audio = true;

	if (strnlen(rom_load_startup, 512) == 0) {
		load_logo();
	}
	else {
		load_cartridge(rom_load_startup);
	}
	
	// UI settings.
	denvergui::denvergui_state windowstates;
	windowstates.show_apu_debugger = false;
	windowstates.mainwin = win;
	windowstates.scaling = display_scale;

	// setup debug rendering for ppu.
	ppu_debug_vram* ppu_visual_debug = new ppu_debug_vram();
	ppu_visual_debug->set_target_ppu(ppu_device);

	// set the debugger callback for the ppu.
	ppu_device->dbg_callback = ([&]() {
		if (windowstates.show_ppu_debugger) {
			ppu_visual_debug->render_nametable();
		}
	});

	// new call back code. set callback to the ppu.
	ppu_device->callback = ([&]() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type) {
			case SDL_QUIT:
				clock.running = false;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if (event.key.keysym.scancode == 0x35) {
					audio->boostspeed = (event.type == SDL_KEYDOWN);
				}
				joydefs->process_kb_event(&event);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			case SDL_CONTROLLERAXISMOTION:
				joydefs->process_controller_event(&event);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEADDED:
				joydefs->process_controller_connect_event(&event);
				break;
			case SDL_DROPFILE:
				SDL_DropEvent* dropData = reinterpret_cast<SDL_DropEvent*>(&event);
				// we have dropped the file. Load it.
				// tell the gui to load it.
				windowstates.romChange = true;
				windowstates.changeRomTo = dropData->file;
			}

		}
		if (!no_gui) {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();
		}
		prepare_frame();
		nes_frame_tex* nesframe = returnFrameAsTexture();

		// ToDo: refactor this into the nes_emulator class (emulator/nes.h)
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		if (!linear_filter) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nesframe->w, nesframe->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)nesframe->texture);

		if (windowstates.show_ppu_debugger) {
			// render ppu debugger pattern table.
			ppu_visual_debug->render_tilemap(windowstates.pattern_palette);
			glBindTexture(GL_TEXTURE_2D, ppu_ptable);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)ppu_visual_debug->get_patterntable_render());

			// render ppu debugger name table.
			ppu_visual_debug->show_scroll_registers = windowstates.show_scroll_regs;
			ppu_visual_debug->show_ppu_updates_from_cpu = windowstates.show_ppu_updates;

			glBindTexture(GL_TEXTURE_2D, ppu_ntable);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)ppu_visual_debug->get_nametable_render());

			windowstates.pattern_tex = ppu_ptable;
			windowstates.ntable_tex = ppu_ntable;
		}

		int w, h;
		SDL_GetWindowSize(win, &w, &h);
		renderFrameToGL(w, h, tex);

		if (!no_gui) {
			denvergui::render_main(this, tex, &windowstates);
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
				SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
			}
		}

		SDL_GL_SwapWindow(win);
		sync_audio();
	});
	std::cout << "Emulation is starting.\n";
	fast_run_callback();
}