; Listing generated by Microsoft (R) Optimizing Compiler Version 19.16.27050.0 

include listing.inc

INCLUDELIB OLDNAMES

PUBLIC	??_C@_0BA@OKGNICPF@Denver?5PPU?5Unit@		; `string'
PUBLIC	??_C@_0P@MFMHDPIH@PPU?5mainram?52k@		; `string'
PUBLIC	??_C@_0BJ@GBIMAOLL@PPU?5palette?5RAM?532?5bytes@ ; `string'
PUBLIC	??_R2ppu@@8					; ppu::`RTTI Base Class Array'
PUBLIC	??_R2ppu_pal_ram@@8				; ppu_pal_ram::`RTTI Base Class Array'
PUBLIC	??_R2ppuram@@8					; ppuram::`RTTI Base Class Array'
PUBLIC	??_R1A@?0A@EA@ppu@@8				; ppu::`RTTI Base Class Descriptor at (0,-1,0,64)'
PUBLIC	??_R1A@?0A@EA@ppu_pal_ram@@8			; ppu_pal_ram::`RTTI Base Class Descriptor at (0,-1,0,64)'
PUBLIC	??_R1A@?0A@EA@ppuram@@8				; ppuram::`RTTI Base Class Descriptor at (0,-1,0,64)'
PUBLIC	??_R3ppu@@8					; ppu::`RTTI Class Hierarchy Descriptor'
PUBLIC	??_R0?AVppu@@@8					; ppu `RTTI Type Descriptor'
PUBLIC	??_R3ppu_pal_ram@@8				; ppu_pal_ram::`RTTI Class Hierarchy Descriptor'
PUBLIC	??_R0?AVppu_pal_ram@@@8				; ppu_pal_ram `RTTI Type Descriptor'
PUBLIC	??_R3ppuram@@8					; ppuram::`RTTI Class Hierarchy Descriptor'
PUBLIC	??_R0?AVppuram@@@8				; ppuram `RTTI Type Descriptor'
PUBLIC	??_R4ppu@@6B@					; ppu::`RTTI Complete Object Locator'
PUBLIC	??_R4ppu_pal_ram@@6B@				; ppu_pal_ram::`RTTI Complete Object Locator'
PUBLIC	??_R4ppuram@@6B@				; ppuram::`RTTI Complete Object Locator'
PUBLIC	??_7ppu_pal_ram@@6B@				; ppu_pal_ram::`vftable'
PUBLIC	??_7ppuram@@6B@					; ppuram::`vftable'
PUBLIC	??_7ppu@@6B@					; ppu::`vftable'
;	COMDAT ??_7ppu@@6B@
CONST	SEGMENT
??_7ppu@@6B@ DQ	FLAT:??_R4ppu@@6B@			; ppu::`vftable'
	DQ	FLAT:?rundevice@ppu@@UEAAHH@Z
	DQ	FLAT:?dma@ppu@@UEAAXPEAE_N1@Z
	DQ	FLAT:??_Eppu@@UEAAPEAXI@Z
	DQ	FLAT:?write@ppu@@UEAAXHHE@Z
	DQ	FLAT:?read@ppu@@UEAAEHH@Z
CONST	ENDS
;	COMDAT ??_7ppuram@@6B@
CONST	SEGMENT
??_7ppuram@@6B@ DQ FLAT:??_R4ppuram@@6B@		; ppuram::`vftable'
	DQ	FLAT:?rundevice@device@@UEAAHH@Z
	DQ	FLAT:?dma@device@@UEAAXPEAE_N1@Z
	DQ	FLAT:??_Eppuram@@UEAAPEAXI@Z
	DQ	FLAT:?write@ppuram@@UEAAXHHE@Z
	DQ	FLAT:?read@ppuram@@UEAAEHH@Z
CONST	ENDS
;	COMDAT ??_7ppu_pal_ram@@6B@
CONST	SEGMENT
??_7ppu_pal_ram@@6B@ DQ FLAT:??_R4ppu_pal_ram@@6B@	; ppu_pal_ram::`vftable'
	DQ	FLAT:?rundevice@device@@UEAAHH@Z
	DQ	FLAT:?dma@device@@UEAAXPEAE_N1@Z
	DQ	FLAT:??_Eppu_pal_ram@@UEAAPEAXI@Z
	DQ	FLAT:?write@ppu_pal_ram@@UEAAXHHE@Z
	DQ	FLAT:?read@ppu_pal_ram@@UEAAEHH@Z
CONST	ENDS
;	COMDAT ??_R4ppuram@@6B@
rdata$r	SEGMENT
??_R4ppuram@@6B@ DD 01H					; ppuram::`RTTI Complete Object Locator'
	DD	00H
	DD	00H
	DD	imagerel ??_R0?AVppuram@@@8
	DD	imagerel ??_R3ppuram@@8
	DD	imagerel ??_R4ppuram@@6B@
rdata$r	ENDS
;	COMDAT ??_R4ppu_pal_ram@@6B@
rdata$r	SEGMENT
??_R4ppu_pal_ram@@6B@ DD 01H				; ppu_pal_ram::`RTTI Complete Object Locator'
	DD	00H
	DD	00H
	DD	imagerel ??_R0?AVppu_pal_ram@@@8
	DD	imagerel ??_R3ppu_pal_ram@@8
	DD	imagerel ??_R4ppu_pal_ram@@6B@
rdata$r	ENDS
;	COMDAT ??_R4ppu@@6B@
rdata$r	SEGMENT
??_R4ppu@@6B@ DD 01H					; ppu::`RTTI Complete Object Locator'
	DD	00H
	DD	00H
	DD	imagerel ??_R0?AVppu@@@8
	DD	imagerel ??_R3ppu@@8
	DD	imagerel ??_R4ppu@@6B@
rdata$r	ENDS
;	COMDAT ??_R0?AVppuram@@@8
data$r	SEGMENT
??_R0?AVppuram@@@8 DQ FLAT:??_7type_info@@6B@		; ppuram `RTTI Type Descriptor'
	DQ	0000000000000000H
	DB	'.?AVppuram@@', 00H
data$r	ENDS
;	COMDAT ??_R3ppuram@@8
rdata$r	SEGMENT
??_R3ppuram@@8 DD 00H					; ppuram::`RTTI Class Hierarchy Descriptor'
	DD	00H
	DD	03H
	DD	imagerel ??_R2ppuram@@8
rdata$r	ENDS
;	COMDAT ??_R0?AVppu_pal_ram@@@8
data$r	SEGMENT
??_R0?AVppu_pal_ram@@@8 DQ FLAT:??_7type_info@@6B@	; ppu_pal_ram `RTTI Type Descriptor'
	DQ	0000000000000000H
	DB	'.?AVppu_pal_ram@@', 00H
data$r	ENDS
;	COMDAT ??_R3ppu_pal_ram@@8
rdata$r	SEGMENT
??_R3ppu_pal_ram@@8 DD 00H				; ppu_pal_ram::`RTTI Class Hierarchy Descriptor'
	DD	00H
	DD	03H
	DD	imagerel ??_R2ppu_pal_ram@@8
rdata$r	ENDS
;	COMDAT ??_R0?AVppu@@@8
data$r	SEGMENT
??_R0?AVppu@@@8 DQ FLAT:??_7type_info@@6B@		; ppu `RTTI Type Descriptor'
	DQ	0000000000000000H
	DB	'.?AVppu@@', 00H
data$r	ENDS
;	COMDAT ??_R3ppu@@8
rdata$r	SEGMENT
??_R3ppu@@8 DD	00H					; ppu::`RTTI Class Hierarchy Descriptor'
	DD	00H
	DD	03H
	DD	imagerel ??_R2ppu@@8
rdata$r	ENDS
;	COMDAT ??_R1A@?0A@EA@ppuram@@8
rdata$r	SEGMENT
??_R1A@?0A@EA@ppuram@@8 DD imagerel ??_R0?AVppuram@@@8	; ppuram::`RTTI Base Class Descriptor at (0,-1,0,64)'
	DD	02H
	DD	00H
	DD	0ffffffffH
	DD	00H
	DD	040H
	DD	imagerel ??_R3ppuram@@8
rdata$r	ENDS
;	COMDAT ??_R1A@?0A@EA@ppu_pal_ram@@8
rdata$r	SEGMENT
??_R1A@?0A@EA@ppu_pal_ram@@8 DD imagerel ??_R0?AVppu_pal_ram@@@8 ; ppu_pal_ram::`RTTI Base Class Descriptor at (0,-1,0,64)'
	DD	02H
	DD	00H
	DD	0ffffffffH
	DD	00H
	DD	040H
	DD	imagerel ??_R3ppu_pal_ram@@8
rdata$r	ENDS
;	COMDAT ??_R1A@?0A@EA@ppu@@8
rdata$r	SEGMENT
??_R1A@?0A@EA@ppu@@8 DD imagerel ??_R0?AVppu@@@8	; ppu::`RTTI Base Class Descriptor at (0,-1,0,64)'
	DD	02H
	DD	00H
	DD	0ffffffffH
	DD	00H
	DD	040H
	DD	imagerel ??_R3ppu@@8
rdata$r	ENDS
;	COMDAT ??_R2ppuram@@8
rdata$r	SEGMENT
??_R2ppuram@@8 DD imagerel ??_R1A@?0A@EA@ppuram@@8	; ppuram::`RTTI Base Class Array'
	DD	imagerel ??_R1A@?0A@EA@bus_device@@8
	DD	imagerel ??_R1A@?0A@EA@device@@8
	ORG $+3
rdata$r	ENDS
;	COMDAT ??_R2ppu_pal_ram@@8
rdata$r	SEGMENT
??_R2ppu_pal_ram@@8 DD imagerel ??_R1A@?0A@EA@ppu_pal_ram@@8 ; ppu_pal_ram::`RTTI Base Class Array'
	DD	imagerel ??_R1A@?0A@EA@bus_device@@8
	DD	imagerel ??_R1A@?0A@EA@device@@8
	ORG $+3
rdata$r	ENDS
;	COMDAT ??_R2ppu@@8
rdata$r	SEGMENT
??_R2ppu@@8 DD	imagerel ??_R1A@?0A@EA@ppu@@8		; ppu::`RTTI Base Class Array'
	DD	imagerel ??_R1A@?0A@EA@bus_device@@8
	DD	imagerel ??_R1A@?0A@EA@device@@8
	ORG $+3
rdata$r	ENDS
;	COMDAT ??_C@_0BJ@GBIMAOLL@PPU?5palette?5RAM?532?5bytes@
CONST	SEGMENT
??_C@_0BJ@GBIMAOLL@PPU?5palette?5RAM?532?5bytes@ DB 'PPU palette RAM 32 b'
	DB	'ytes', 00H					; `string'
CONST	ENDS
;	COMDAT ??_C@_0P@MFMHDPIH@PPU?5mainram?52k@
CONST	SEGMENT
??_C@_0P@MFMHDPIH@PPU?5mainram?52k@ DB 'PPU mainram 2k', 00H ; `string'
CONST	ENDS
;	COMDAT ??_C@_0BA@OKGNICPF@Denver?5PPU?5Unit@
CONST	SEGMENT
??_C@_0BA@OKGNICPF@Denver?5PPU?5Unit@ DB 'Denver PPU Unit', 00H ; `string'
PUBLIC	?read@ppu_pal_ram@@UEAAEHH@Z			; ppu_pal_ram::read
PUBLIC	?write@ppu_pal_ram@@UEAAXHHE@Z			; ppu_pal_ram::write
PUBLIC	?pal_addr_compute@ppu_pal_ram@@AEAAHH@Z		; ppu_pal_ram::pal_addr_compute
PUBLIC	??1ppu_pal_ram@@UEAA@XZ				; ppu_pal_ram::~ppu_pal_ram
PUBLIC	??_Gppu_pal_ram@@UEAAPEAXI@Z			; ppu_pal_ram::`scalar deleting destructor'
PUBLIC	??0ppu_pal_ram@@QEAA@XZ				; ppu_pal_ram::ppu_pal_ram
PUBLIC	?read@ppuram@@UEAAEHH@Z				; ppuram::read
PUBLIC	?write@ppuram@@UEAAXHHE@Z			; ppuram::write
PUBLIC	??1ppuram@@UEAA@XZ				; ppuram::~ppuram
PUBLIC	??_Gppuram@@UEAAPEAXI@Z				; ppuram::`scalar deleting destructor'
PUBLIC	??0ppuram@@QEAA@XZ				; ppuram::ppuram
PUBLIC	?dma@ppu@@UEAAXPEAE_N1@Z			; ppu::dma
PUBLIC	?rundevice@ppu@@UEAAHH@Z			; ppu::rundevice
PUBLIC	?write@ppu@@UEAAXHHE@Z				; ppu::write
PUBLIC	?read@ppu@@UEAAEHH@Z				; ppu::read
PUBLIC	??1ppu@@UEAA@XZ					; ppu::~ppu
PUBLIC	??_Gppu@@UEAAPEAXI@Z				; ppu::`scalar deleting destructor'
PUBLIC	??0ppu@@QEAA@XZ					; ppu::ppu
PUBLIC	?set_char_rom@ppu@@QEAAXPEAVbus_device@@@Z	; ppu::set_char_rom
PUBLIC	?configure_horizontal_mirror@ppu@@QEAAXXZ	; ppu::configure_horizontal_mirror
PUBLIC	?isFrameReady@ppu@@QEAA_NXZ			; ppu::isFrameReady
PUBLIC	?getFrameBuffer@ppu@@QEAAPEAXXZ			; ppu::getFrameBuffer
EXTRN	??_Eppuram@@UEAAPEAXI@Z:PROC			; ppuram::`vector deleting destructor'
EXTRN	??_Eppu_pal_ram@@UEAAPEAXI@Z:PROC		; ppu_pal_ram::`vector deleting destructor'
EXTRN	??_Eppu@@UEAAPEAXI@Z:PROC			; ppu::`vector deleting destructor'
;	COMDAT pdata
pdata	SEGMENT
$pdata$??1ppu_pal_ram@@UEAA@XZ DD imagerel $LN10@ppu_pal_ra
	DD	imagerel $LN10@ppu_pal_ra+55
	DD	imagerel $unwind$??1ppu_pal_ram@@UEAA@XZ
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??_Gppu_pal_ram@@UEAAPEAXI@Z DD imagerel $LN15@scalar
	DD	imagerel $LN15@scalar+87
	DD	imagerel $unwind$??_Gppu_pal_ram@@UEAAPEAXI@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??0ppu_pal_ram@@QEAA@XZ DD imagerel $LN7@ppu_pal_ra
	DD	imagerel $LN7@ppu_pal_ra+91
	DD	imagerel $unwind$??0ppu_pal_ram@@QEAA@XZ
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??1ppuram@@UEAA@XZ DD imagerel $LN10@ppuram
	DD	imagerel $LN10@ppuram+55
	DD	imagerel $unwind$??1ppuram@@UEAA@XZ
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??_Gppuram@@UEAAPEAXI@Z DD imagerel $LN15@scalar
	DD	imagerel $LN15@scalar+87
	DD	imagerel $unwind$??_Gppuram@@UEAAPEAXI@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??0ppuram@@QEAA@XZ DD imagerel $LN7@ppuram
	DD	imagerel $LN7@ppuram+91
	DD	imagerel $unwind$??0ppuram@@QEAA@XZ
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?rundevice@ppu@@UEAAHH@Z DD imagerel $LN148@rundevice
	DD	imagerel $LN148@rundevice+40
	DD	imagerel $unwind$?rundevice@ppu@@UEAAHH@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$3$?rundevice@ppu@@UEAAHH@Z DD imagerel $LN148@rundevice+40
	DD	imagerel $LN148@rundevice+2827
	DD	imagerel $chain$3$?rundevice@ppu@@UEAAHH@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$4$?rundevice@ppu@@UEAAHH@Z DD imagerel $LN148@rundevice+2827
	DD	imagerel $LN148@rundevice+2839
	DD	imagerel $chain$4$?rundevice@ppu@@UEAAHH@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?write@ppu@@UEAAXHHE@Z DD imagerel $LN19@write
	DD	imagerel $LN19@write+619
	DD	imagerel $unwind$?write@ppu@@UEAAXHHE@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?read@ppu@@UEAAEHH@Z DD imagerel $LN15@read
	DD	imagerel $LN15@read+152
	DD	imagerel $unwind$?read@ppu@@UEAAEHH@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$0$?read@ppu@@UEAAEHH@Z DD imagerel $LN15@read+152
	DD	imagerel $LN15@read+220
	DD	imagerel $chain$0$?read@ppu@@UEAAEHH@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$1$?read@ppu@@UEAAEHH@Z DD imagerel $LN15@read+220
	DD	imagerel $LN15@read+228
	DD	imagerel $chain$1$?read@ppu@@UEAAEHH@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??1ppu@@UEAA@XZ DD imagerel $LN248@ppu
	DD	imagerel $LN248@ppu+341
	DD	imagerel $unwind$??1ppu@@UEAA@XZ
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??_Gppu@@UEAAPEAXI@Z DD imagerel $LN253@scalar
	DD	imagerel $LN253@scalar+380
	DD	imagerel $unwind$??_Gppu@@UEAAPEAXI@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??0ppu@@QEAA@XZ DD imagerel $LN189@ppu
	DD	imagerel $LN189@ppu+439
	DD	imagerel $unwind$??0ppu@@QEAA@XZ
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$?set_char_rom@ppu@@QEAAXPEAVbus_device@@@Z DD imagerel $LN195@set_char_r
	DD	imagerel $LN195@set_char_r+177
	DD	imagerel $unwind$?set_char_rom@ppu@@QEAAXPEAVbus_device@@@Z
pdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?set_char_rom@ppu@@QEAAXPEAVbus_device@@@Z DD 041919H
	DD	09340aH
	DD	07006520aH
	DD	imagerel __GSHandlerCheck
	DD	028H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$ip2state$??0ppu@@QEAA@XZ DD imagerel ??0ppu@@QEAA@XZ+339
	DD	03H
	DD	imagerel ??0ppu@@QEAA@XZ+417
	DD	0ffffffffH
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$stateUnwindMap$??0ppu@@QEAA@XZ DD 0ffffffffH
	DD	imagerel ?dtor$0@?0???0ppu@@QEAA@XZ@4HA
	DD	00H
	DD	imagerel ?dtor$1@?0???0ppu@@QEAA@XZ@4HA
	DD	01H
	DD	imagerel ?dtor$2@?0???0ppu@@QEAA@XZ@4HA
	DD	02H
	DD	imagerel ?dtor$3@?0???0ppu@@QEAA@XZ@4HA
xdata	ENDS
;	COMDAT CONST
CONST	SEGMENT
$cppxdata$??0ppu@@QEAA@XZ DQ 00000000419930522r	; 8.69997e-314
	DD	imagerel $stateUnwindMap$??0ppu@@QEAA@XZ
	DQ	00000000000000000r		; 0
	DD	02H
	DD	imagerel $ip2state$??0ppu@@QEAA@XZ
	DQ	00000000000000020r		; 1.58101e-322
	DD	01H
CONST	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??0ppu@@QEAA@XZ DD 081c11H
	DD	0e541cH
	DD	0d3417H
	DD	0e0057209H
	DD	060027003H
	DD	imagerel __CxxFrameHandler3
	DD	imagerel $cppxdata$??0ppu@@QEAA@XZ
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??_Gppu@@UEAAPEAXI@Z DD 081401H
	DD	096414H
	DD	085414H
	DD	073414H
	DD	070103214H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??1ppu@@UEAA@XZ DD 060f01H
	DD	08640fH
	DD	07340fH
	DD	0700b320fH
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$chain$1$?read@ppu@@UEAAEHH@Z DD 021H
	DD	imagerel $LN15@read
	DD	imagerel $LN15@read+152
	DD	imagerel $unwind$?read@ppu@@UEAAEHH@Z
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$chain$0$?read@ppu@@UEAAEHH@Z DD 020521H
	DD	063405H
	DD	imagerel $LN15@read
	DD	imagerel $LN15@read+152
	DD	imagerel $unwind$?read@ppu@@UEAAEHH@Z
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?read@ppu@@UEAAEHH@Z DD 020601H
	DD	070023206H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?write@ppu@@UEAAXHHE@Z DD 060f01H
	DD	07640fH
	DD	06340fH
	DD	0700b320fH
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$chain$4$?rundevice@ppu@@UEAAHH@Z DD 021H
	DD	imagerel $LN148@rundevice
	DD	imagerel $LN148@rundevice+40
	DD	imagerel $unwind$?rundevice@ppu@@UEAAHH@Z
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$chain$3$?rundevice@ppu@@UEAAHH@Z DD 082221H
	DD	04f422H
	DD	0cd417H
	DD	0bc40dH
	DD	0a7405H
	DD	imagerel $LN148@rundevice
	DD	imagerel $LN148@rundevice+40
	DD	imagerel $unwind$?rundevice@ppu@@UEAAHH@Z
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$?rundevice@ppu@@UEAAHH@Z DD 050a01H
	DD	0e006420aH
	DD	050036004H
	DD	03002H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??0ppuram@@QEAA@XZ DD 020601H
	DD	030023206H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??_Gppuram@@UEAAPEAXI@Z DD 040a01H
	DD	06340aH
	DD	07006320aH
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??1ppuram@@UEAA@XZ DD 020601H
	DD	030023206H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??0ppu_pal_ram@@QEAA@XZ DD 020601H
	DD	030023206H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??_Gppu_pal_ram@@UEAAPEAXI@Z DD 040a01H
	DD	06340aH
	DD	07006320aH
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??1ppu_pal_ram@@UEAA@XZ DD 020601H
	DD	030023206H
END
