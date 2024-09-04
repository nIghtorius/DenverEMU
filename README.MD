Denver project
--------------
This is my first open source project. I have tested this on Windows x86/x64 and Linux x64.


What is it?
-----------
It is a open source NES emulator. It is far from perfect. There are some small emulation bugs (that needs to be squashed). 
Only supports a handful of mappers. This will be expanded in the feature.

NESDB
-----
Since 17-08-2024, Denver has added support for the NES20DB database. But Denver accepts the database only in binary format. The format is simple. You can look in the source under helpers/nesdb.h in how the binary format is expected.

I have included a binary format of the nes20db database in this repo. /nes20db.bin. If you want to make use of it, make sure that the file resides in the same folder as the binary. Upon starting Denver. It will look for this file and load it when it is found.

Famicom Disk System (FDS)
-------------------------
With codebases of 04-09-2024 and more recent. Denver now supports FDS emulation. 
Requirements FDS emulation:
* You need to find the FDS BIOS (8KB in size) and save the file as disksys.rom in the location of the Denver executable/binary.

What works with FDS:
* I have tried to implement a sturdy FDS HW emulation, it passes all the tests in fdsirqtests.fds
* It can load and write to disks. 
* Data that has been written will not be written in the FDS file. But a second file filename.fds.dwd will be written. Just as the .SRM files with battery backed cartridge roms. 
* If you want to "reset" your FDS image. You only need to remove the .dwd file.
* When loading FDS files. A extra menu option in the GUI will appear, namely: FDS. There you can swap sides and disks.

Does it support NSF?
--------------------
Yes it does support NSF. The compability is suprisingly great. It plays most of the NSFs. 
It does so by "emulating" a NSF player on a NES. mapper_nsf cart type.
Currently does the following expansion audio:
* VRC6
* VRC7
* Namco 163
* Sunsoft 5B
* MMC5

Denver also supports NSFe files. Which also have support for named tracks / track length, etc.

Expansion audio will be expanded later on.

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
--no-expanded-adio	, disables all audio expansions.
--no-gui		, disables the GUI elements (pure mode), it is required to supply a rom file with the --rom-file switch.
--rom-file		, supply rom file to load on startup.
--fullscreen		, starts denver in exclusive full screen mode. be sure to define width and height.
--width	<width>		, supply width (ex: 1920)
--height <height>	, supply height (ex: 1080)
--override-scale <scale>, scales the GUI. 1 is normal. 2 is twice as big. When not supplied. Denver will compute depending on the DPI settings of the display.
--show-controllers	, shows the detected game controllers and exits.
--select-controller-id-port1, select which game controller needs to be connected on game port 1
--select-controller-id-port2, select which game controller needs to be connected on game port 1
--executiontrace	, writes a execution log (use with --rom-file) of what the virtual cpu is doing. Files tend to get big fast.

