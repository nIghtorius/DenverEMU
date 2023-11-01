Denver project
--------------
This is my first open source project. I have tested this on Windows x86/x64 and Linux x64.


What is it?
-----------
It is a open source NES emulator. It is far from perfect. There are some small emulation bugs (that needs to be squashed). 
Only supports 4 mappers (5 cart types) will be expanded in the future.


How to compile?
---------------
In Windows. Open the solution file with Visual Studio. Download the headers and libraries for SDL2. Modify the paths in the project configuration to point to your SDL2 directories and you are good to go.
In Linux. Download from SDL2 lib from git and compile and install this. Then you goto the project directory and type: make

the Makefile script will compile all the files and link it together. You can then run the ./denver binary.


Command-line switches
---------------------
--vsync			, enables vsync
--no-vsync		, disabled vsync ( on some linux distro's this is ignored you can use "export vblank_mode=0" command in terminal to perma disable vsync )
--no-audio		, disables the audio output (APU is still emulated)
			  per default (when no vsync, denver times the emulation speed on audio, by disabling audio you essentially make denver run on uncapped speed
			  to make it run in pace. Be sure to set  your display to 60hz and use the --vsync flag)
--no-apu-emulation	, disables all audio and audio emulation. APU is not emulated. Denver will run uncapped so make sure vsync enabled with 60hz display.
