; Listing generated by Microsoft (R) Optimizing Compiler Version 19.16.27050.0 

include listing.inc

INCLUDELIB OLDNAMES

PUBLIC	??_C@_0P@IDHECIDN@NES?5APU?5Device@		; `string'
PUBLIC	??_R2apu@@8					; apu::`RTTI Base Class Array'
PUBLIC	??_R2bus_device@@8				; bus_device::`RTTI Base Class Array'
PUBLIC	??_R2device@@8					; device::`RTTI Base Class Array'
PUBLIC	??_R1A@?0A@EA@apu@@8				; apu::`RTTI Base Class Descriptor at (0,-1,0,64)'
PUBLIC	??_R1A@?0A@EA@bus_device@@8			; bus_device::`RTTI Base Class Descriptor at (0,-1,0,64)'
PUBLIC	??_R1A@?0A@EA@device@@8				; device::`RTTI Base Class Descriptor at (0,-1,0,64)'
PUBLIC	??_R3bus_device@@8				; bus_device::`RTTI Class Hierarchy Descriptor'
PUBLIC	??_R0?AVbus_device@@@8				; bus_device `RTTI Type Descriptor'
PUBLIC	??_R3device@@8					; device::`RTTI Class Hierarchy Descriptor'
PUBLIC	??_R0?AVdevice@@@8				; device `RTTI Type Descriptor'
PUBLIC	??_R3apu@@8					; apu::`RTTI Class Hierarchy Descriptor'
PUBLIC	??_R0?AVapu@@@8					; apu `RTTI Type Descriptor'
PUBLIC	??_R4apu@@6B@					; apu::`RTTI Complete Object Locator'
PUBLIC	??_7apu@@6B@					; apu::`vftable'
EXTRN	??_7type_info@@6B@:BYTE				; type_info::`vftable'
EXTRN	__imp_strcpy_s:PROC
EXTRN	??3@YAXPEAX_K@Z:PROC				; operator delete
EXTRN	__imp___std_terminate:PROC
EXTRN	__security_check_cookie:PROC
EXTRN	__imp___CxxFrameHandler3:PROC
CONST	ENDS
;	COMDAT ??_7apu@@6B@
CONST	SEGMENT
??_7apu@@6B@ DQ	FLAT:??_R4apu@@6B@			; apu::`vftable'
	DQ	FLAT:?rundevice@apu@@UEAAHH@Z
	DQ	FLAT:?dma@device@@UEAAXPEAE_N1@Z
	DQ	FLAT:??_Eapu@@UEAAPEAXI@Z
	DQ	FLAT:?write@apu@@UEAAXHHE@Z
	DQ	FLAT:?read@apu@@UEAAEHH@Z
CONST	ENDS
;	COMDAT ??_R4apu@@6B@
rdata$r	SEGMENT
??_R4apu@@6B@ DD 01H					; apu::`RTTI Complete Object Locator'
	DD	00H
	DD	00H
	DD	imagerel ??_R0?AVapu@@@8
	DD	imagerel ??_R3apu@@8
	DD	imagerel ??_R4apu@@6B@
rdata$r	ENDS
;	COMDAT ??_R0?AVapu@@@8
data$r	SEGMENT
??_R0?AVapu@@@8 DQ FLAT:??_7type_info@@6B@		; apu `RTTI Type Descriptor'
	DQ	0000000000000000H
	DB	'.?AVapu@@', 00H
data$r	ENDS
;	COMDAT ??_R3apu@@8
rdata$r	SEGMENT
??_R3apu@@8 DD	00H					; apu::`RTTI Class Hierarchy Descriptor'
	DD	00H
	DD	03H
	DD	imagerel ??_R2apu@@8
rdata$r	ENDS
;	COMDAT ??_R0?AVdevice@@@8
data$r	SEGMENT
??_R0?AVdevice@@@8 DQ FLAT:??_7type_info@@6B@		; device `RTTI Type Descriptor'
	DQ	0000000000000000H
	DB	'.?AVdevice@@', 00H
data$r	ENDS
;	COMDAT ??_R3device@@8
rdata$r	SEGMENT
??_R3device@@8 DD 00H					; device::`RTTI Class Hierarchy Descriptor'
	DD	00H
	DD	01H
	DD	imagerel ??_R2device@@8
rdata$r	ENDS
;	COMDAT ??_R0?AVbus_device@@@8
data$r	SEGMENT
??_R0?AVbus_device@@@8 DQ FLAT:??_7type_info@@6B@	; bus_device `RTTI Type Descriptor'
	DQ	0000000000000000H
	DB	'.?AVbus_device@@', 00H
data$r	ENDS
;	COMDAT ??_R3bus_device@@8
rdata$r	SEGMENT
??_R3bus_device@@8 DD 00H				; bus_device::`RTTI Class Hierarchy Descriptor'
	DD	00H
	DD	02H
	DD	imagerel ??_R2bus_device@@8
rdata$r	ENDS
;	COMDAT ??_R1A@?0A@EA@device@@8
rdata$r	SEGMENT
??_R1A@?0A@EA@device@@8 DD imagerel ??_R0?AVdevice@@@8	; device::`RTTI Base Class Descriptor at (0,-1,0,64)'
	DD	00H
	DD	00H
	DD	0ffffffffH
	DD	00H
	DD	040H
	DD	imagerel ??_R3device@@8
rdata$r	ENDS
;	COMDAT ??_R1A@?0A@EA@bus_device@@8
rdata$r	SEGMENT
??_R1A@?0A@EA@bus_device@@8 DD imagerel ??_R0?AVbus_device@@@8 ; bus_device::`RTTI Base Class Descriptor at (0,-1,0,64)'
	DD	01H
	DD	00H
	DD	0ffffffffH
	DD	00H
	DD	040H
	DD	imagerel ??_R3bus_device@@8
rdata$r	ENDS
;	COMDAT ??_R1A@?0A@EA@apu@@8
rdata$r	SEGMENT
??_R1A@?0A@EA@apu@@8 DD imagerel ??_R0?AVapu@@@8	; apu::`RTTI Base Class Descriptor at (0,-1,0,64)'
	DD	02H
	DD	00H
	DD	0ffffffffH
	DD	00H
	DD	040H
	DD	imagerel ??_R3apu@@8
rdata$r	ENDS
;	COMDAT ??_R2device@@8
rdata$r	SEGMENT
??_R2device@@8 DD imagerel ??_R1A@?0A@EA@device@@8	; device::`RTTI Base Class Array'
	ORG $+3
rdata$r	ENDS
;	COMDAT ??_R2bus_device@@8
rdata$r	SEGMENT
??_R2bus_device@@8 DD imagerel ??_R1A@?0A@EA@bus_device@@8 ; bus_device::`RTTI Base Class Array'
	DD	imagerel ??_R1A@?0A@EA@device@@8
	ORG $+3
rdata$r	ENDS
;	COMDAT ??_R2apu@@8
rdata$r	SEGMENT
??_R2apu@@8 DD	imagerel ??_R1A@?0A@EA@apu@@8		; apu::`RTTI Base Class Array'
	DD	imagerel ??_R1A@?0A@EA@bus_device@@8
	DD	imagerel ??_R1A@?0A@EA@device@@8
	ORG $+3
rdata$r	ENDS
;	COMDAT ??_C@_0P@IDHECIDN@NES?5APU?5Device@
CONST	SEGMENT
??_C@_0P@IDHECIDN@NES?5APU?5Device@ DB 'NES APU Device', 00H ; `string'
CONST	ENDS
PUBLIC	?rundevice@apu@@UEAAHH@Z			; apu::rundevice
PUBLIC	?write@apu@@UEAAXHHE@Z				; apu::write
PUBLIC	?read@apu@@UEAAEHH@Z				; apu::read
PUBLIC	??1apu@@UEAA@XZ					; apu::~apu
PUBLIC	??_Gapu@@UEAAPEAXI@Z				; apu::`scalar deleting destructor'
PUBLIC	??0apu@@QEAA@XZ					; apu::apu
EXTRN	??_Eapu@@UEAAPEAXI@Z:PROC			; apu::`vector deleting destructor'
EXTRN	_CxxThrowException:PROC
EXTRN	__CxxFrameHandler3:PROC
EXTRN	__GSHandlerCheck:PROC
EXTRN	__GSHandlerCheck_EH:PROC
EXTRN	__std_terminate:PROC
EXTRN	memcpy:PROC
EXTRN	memmove:PROC
EXTRN	memset:PROC
EXTRN	__ImageBase:BYTE
EXTRN	__security_cookie:QWORD
;	COMDAT pdata
pdata	SEGMENT
$pdata$??_Gapu@@UEAAPEAXI@Z DD imagerel $LN15
	DD	imagerel $LN15+67
	DD	imagerel $unwind$??_Gapu@@UEAAPEAXI@Z
pdata	ENDS
;	COMDAT pdata
pdata	SEGMENT
$pdata$??0apu@@QEAA@XZ DD imagerel $LN7
	DD	imagerel $LN7+91
	DD	imagerel $unwind$??0apu@@QEAA@XZ
;	COMDAT xdata
xdata	SEGMENT
$unwind$??0apu@@QEAA@XZ DD 020601H
	DD	030023206H
xdata	ENDS
;	COMDAT xdata
xdata	SEGMENT
$unwind$??_Gapu@@UEAAPEAXI@Z DD 040a01H
	DD	06340aH
	DD	07006320aH
xdata	ENDS
; Function compile flags: /Ogtpy
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\audio\apu.cpp
;	COMDAT ??0apu@@QEAA@XZ
_TEXT	SEGMENT
this$ = 48
??0apu@@QEAA@XZ PROC					; apu::apu, COMDAT

; 4    : apu::apu() {

$LN7:
	push	rbx
	sub	rsp, 32					; 00000020H
	mov	rbx, rcx
	call	??0bus_device@@QEAA@XZ			; bus_device::bus_device

; 5    : 	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "NES APU Device");

	mov	rcx, QWORD PTR [rbx+8]
	lea	rax, OFFSET FLAT:??_7apu@@6B@
	lea	r8, OFFSET FLAT:??_C@_0P@IDHECIDN@NES?5APU?5Device@
	mov	QWORD PTR [rbx], rax
	mov	edx, 128				; 00000080H
	call	QWORD PTR __imp_strcpy_s

; 6    : 	devicestart = 0x4000;

	mov	DWORD PTR [rbx+44], 16384		; 00004000H

; 7    : 	deviceend = 0x401F;
; 8    : 	devicemask = 0x401F;
; 9    : 
; 10   : 	// initialize pulse
; 11   : 	pulse[0].pulse2 = false;
; 12   : 	pulse[1].pulse2 = true;
; 13   : 
; 14   : 	// clocking info.
; 15   : 	tick_rate = 3;	// make it same as cpu. tick_rate is a divider against tick_rate 1
; 16   : }

	mov	rax, rbx
	mov	DWORD PTR [rbx+48], 16415		; 0000401fH
	mov	DWORD PTR [rbx+52], 16415		; 0000401fH
	mov	BYTE PTR [rbx+92], 0
	mov	BYTE PTR [rbx+114], 1
	mov	DWORD PTR [rbx+16], 3
	add	rsp, 32					; 00000020H
	pop	rbx
	ret	0
??0apu@@QEAA@XZ ENDP					; apu::apu
_TEXT	ENDS
; Function compile flags: /Ogtpy
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\bus\bus.cpp
;	COMDAT ??_Gapu@@UEAAPEAXI@Z
_TEXT	SEGMENT
this$ = 48
__flags$ = 56
??_Gapu@@UEAAPEAXI@Z PROC				; apu::`scalar deleting destructor', COMDAT
$LN15:
	mov	QWORD PTR [rsp+8], rbx
	push	rdi
	sub	rsp, 32					; 00000020H

; 185  : device::~device() {

	lea	rax, OFFSET FLAT:??_7device@@6B@
	mov	rdi, rcx
	mov	QWORD PTR [rcx], rax
	mov	ebx, edx

; 186  : 	free(devicedescriptor);	// be done with it.

	mov	rcx, QWORD PTR [rcx+8]
	call	QWORD PTR __imp_free
	test	bl, 1
	je	SHORT $LN12@scalar
	mov	edx, 120				; 00000078H
	mov	rcx, rdi
	call	??3@YAXPEAX_K@Z				; operator delete
$LN12@scalar:
	mov	rbx, QWORD PTR [rsp+48]
	mov	rax, rdi
	add	rsp, 32					; 00000020H
	pop	rdi
	ret	0
??_Gapu@@UEAAPEAXI@Z ENDP				; apu::`scalar deleting destructor'
_TEXT	ENDS
; Function compile flags: /Ogtpy
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\audio\apu.cpp
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\bus\bus.cpp
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\audio\apu.cpp
;	COMDAT ??1apu@@UEAA@XZ
_TEXT	SEGMENT
this$ = 8
??1apu@@UEAA@XZ PROC					; apu::~apu, COMDAT
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\bus\bus.cpp

; 185  : device::~device() {

	lea	rax, OFFSET FLAT:??_7device@@6B@
	mov	QWORD PTR [rcx], rax

; 186  : 	free(devicedescriptor);	// be done with it.

	mov	rcx, QWORD PTR [rcx+8]
	rex_jmp	QWORD PTR __imp_free
??1apu@@UEAA@XZ ENDP					; apu::~apu
_TEXT	ENDS
; Function compile flags: /Ogtpy
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\audio\apu.cpp
;	COMDAT ?read@apu@@UEAAEHH@Z
_TEXT	SEGMENT
this$ = 8
addr$ = 16
addr_from_base$ = 24
?read@apu@@UEAAEHH@Z PROC				; apu::read, COMDAT

; 23   : 	return BUS_OPEN_BUS;

	mov	al, 240					; 000000f0H

; 24   : }

	ret	0
?read@apu@@UEAAEHH@Z ENDP				; apu::read
_TEXT	ENDS
; Function compile flags: /Ogtpy
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\audio\apu.cpp
;	COMDAT ?write@apu@@UEAAXHHE@Z
_TEXT	SEGMENT
this$ = 8
addr$ = 16
addr_from_base$ = 24
data$ = 32
?write@apu@@UEAAXHHE@Z PROC				; apu::write, COMDAT

; 26   : void	apu::write(int addr, int addr_from_base, byte data) {

	mov	r11, rcx

; 27   : 	if (addr_from_base < 0x08) {

	cmp	r8d, 8
	jge	$LN2@write

; 28   : 		// pulse channels.
; 29   : 		int pulse_sel = (addr_from_base & PULSE2) > 0;

	mov	r10d, r8d

; 30   : 		byte cmd = addr_from_base & 0x03;

	movzx	edx, r8b
	and	r10d, 4
	and	edx, 3

; 31   : 		switch (cmd) {

	je	$LN5@write
	sub	edx, 1
	je	SHORT $LN6@write
	sub	edx, 1
	je	SHORT $LN7@write
	cmp	edx, 1
	jne	$LN2@write

; 48   : 			break;
; 49   : 		case PULSE_LCL_TIMER:
; 50   : 			pulse[pulse_sel].timer = (pulse[pulse_sel].timer & 0x00FF) | ((data & 0x07) << 8);

	mov	eax, 100				; 00000064H
	test	r10d, r10d
	mov	edx, 78					; 0000004eH
	cmovne	edx, eax
	mov	eax, 255				; 000000ffH
	add	rdx, rcx
	and	WORD PTR [rdx], ax
	movzx	eax, r9b
	and	al, 7

; 51   : 			pulse[pulse_sel].length_counter = (data & 0xF8) >> 3;

	shr	r9b, 3
	movzx	ecx, al
	mov	eax, 82					; 00000052H
	shl	cx, 8
	or	WORD PTR [rdx], cx
	mov	ecx, 104				; 00000068H
	test	r10d, r10d
	cmovne	eax, ecx
	mov	BYTE PTR [rax+r11], r9b

; 52   : 			break;
; 53   : 		}
; 54   : 	}
; 55   : }

	ret	0
$LN7@write:

; 45   : 			break;
; 46   : 		case PULSE_TIMER:
; 47   : 			pulse[pulse_sel].timer = (pulse[pulse_sel].timer & 0x0700) | data;

	test	r10d, r10d
	mov	eax, 100				; 00000064H
	mov	edx, 78					; 0000004eH
	cmovne	edx, eax
	mov	eax, 1792				; 00000700H
	add	rdx, r11
	and	WORD PTR [rdx], ax
	movzx	eax, r9b
	or	WORD PTR [rdx], ax

; 52   : 			break;
; 53   : 		}
; 54   : 	}
; 55   : }

	ret	0
$LN6@write:

; 38   : 			break;
; 39   : 		case PULSE_SWEEP:
; 40   : 			pulse[pulse_sel].sweep_enable = (data & 0x80) > 0;

	xor	edx, edx
	test	r10d, r10d
	setne	dl
	imul	rcx, rdx, 22
	add	rcx, r11
	test	r9b, 128				; 00000080H
	seta	al
	mov	BYTE PTR [rcx+83], al

; 41   : 			pulse[pulse_sel].sweep_divider = (data & 0x70) >> 4;

	movzx	eax, r9b
	shr	al, 4
	and	al, 7

; 42   : 			pulse[pulse_sel].sweep_negate = (data & 0x08) > 0;

	test	r9b, 8
	mov	BYTE PTR [rcx+84], al
	seta	al

; 43   : 			pulse[pulse_sel].sweep_shift = (data & 0x07);

	and	r9b, 7
	mov	BYTE PTR [rcx+86], al

; 44   : 			pulse[pulse_sel].sweep_reload = true;

	lea	rax, QWORD PTR [rdx+4]
	mov	BYTE PTR [rcx+87], r9b
	imul	rcx, rax, 22
	mov	BYTE PTR [rcx+r11], 1

; 52   : 			break;
; 53   : 		}
; 54   : 	}
; 55   : }

	ret	0
$LN5@write:

; 32   : 		case PULSE_DUTY_CYCLE_LCH_VOLENV:
; 33   : 			pulse[pulse_sel].duty_cycle = (data & 0xC0) >> 6;

	movzx	ecx, r9b
	mov	edx, 94					; 0000005eH
	shr	cl, 6
	mov	eax, 72					; 00000048H
	test	r10d, r10d
	cmovne	eax, edx

; 34   : 			pulse[pulse_sel].envelope_loop = (data & 0x20) > 0;

	test	r9b, 32					; 00000020H
	mov	edx, 96					; 00000060H
	mov	BYTE PTR [rax+r11], cl
	seta	cl
	test	r10d, r10d
	mov	eax, 74					; 0000004aH
	cmovne	eax, edx

; 35   : 			pulse[pulse_sel].constant_volume = (data & 0x10) > 0;

	test	r9b, 16
	mov	edx, 97					; 00000061H
	mov	BYTE PTR [rax+r11], cl
	seta	cl
	test	r10d, r10d
	mov	eax, 75					; 0000004bH
	cmovne	eax, edx

; 36   : 			pulse[pulse_sel].volume_envelope = data & 0x0F;

	and	r9b, 15
	test	r10d, r10d
	mov	BYTE PTR [rax+r11], cl
	mov	ecx, 98					; 00000062H
	mov	eax, 76					; 0000004cH
	cmovne	eax, ecx

; 37   : 			pulse[pulse_sel].envelope_reload = true;

	mov	ecx, 111				; 0000006fH
	mov	BYTE PTR [rax+r11], r9b
	mov	eax, 89					; 00000059H
	cmovne	eax, ecx
	mov	BYTE PTR [rax+r11], 1
$LN2@write:

; 52   : 			break;
; 53   : 		}
; 54   : 	}
; 55   : }

	ret	0
?write@apu@@UEAAXHHE@Z ENDP				; apu::write
_TEXT	ENDS
; Function compile flags: /Ogtpy
; File c:\users\nightorius.phibian\source\repos\denveremu\denveremu\audio\apu.cpp
;	COMDAT ?rundevice@apu@@UEAAHH@Z
_TEXT	SEGMENT
this$ = 8
ticks$ = 16
?rundevice@apu@@UEAAHH@Z PROC				; apu::rundevice, COMDAT

; 58   : 	return ticks;

	mov	eax, edx

; 59   : }

	ret	0
?rundevice@apu@@UEAAHH@Z ENDP				; apu::rundevice
_TEXT	ENDS
END
