
;------------------------------------------------------------------------------
;
; Copyright 2024-2025 Rodney Rushing
;
;
; This file is part of VicMatrix for Commodore Color Computers.
;
; MATRIX is free software: you can redistribute it and/or modify it under the
; terms of the GNU General Public License as published by the Free Software
; Foundation, either version 3 of the License, or (at your option) any later
; version.
;
; MATRIX is distributed in the hope that it will be useful, but WITHOUT ANY
; WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
; A PARTICULAR PURPOSE. See the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along with
; MATRIX. If not, see <https://www.gnu.org/licenses/>.
;

;----------------------------------
; SYSTEM FILE FOR C128 40 COLUMN
;

!src "cbm_launcher.inc"

!zone C128 {

;----------------------------------
; BASIC ENTRY POINT
;
	      * = $1C01
		+BLAUNCH .SYSEND  ; Generate BASIC launcher program that
				      ; jumps to the end of this zone.


;----------------------------------
; SYSTEM CONSTANTS
;
.BLACK		=	0
.WHITE		=	1
.RED		=	2
.CYAN		=	3
.PURPLE		=	4
.GREEN		=	5
.BLUE		=	6
.YELLOW		=	7
.ORANGE		=	8
.BROWN		=	9
.LTRED		=	10
.DARKGRAY	=	11
.GRAY		=	12
.LTGREEN	=	13
.LTBLUE		=	14
.LTGRAY		=	15

.BORDER		=	.BLACK
.BACKGND	=	.BLACK
.STKEY		=	$91
.CHROUT		=	$ffd2
.RSTVEC		=	$fffc

; Random number generation
.SIDNOISE	=	$d41b
.SIDFQ3LO	=	$d40e
.SIDFQ3HI	=	$d40f
.SIDCTRL3	=	$d412

; VIC-II Configuration
.VIC2BRDR	=	$d020
.VIC2BGND	=	$d021

.VM1		=	$0A2C

;----------------------------------
; GLOBAL SYSTEM ABSTRACTION (see main.asm for descriptions.)
;

CFGSLOW		=	0
CFGLAYERS	=	2
CFGFADE		=	1

SYSROWS		=	25		; default
SYSCOLS		=	40		; default
SYSVXRAM		=	$0400		; default
SYSCLRAM	=	$D800		; default
SYSCHRAM	=	$3000

T1		=	$63
T2		=	$65
T3		=	$67
SYSWRAND	=	$69
SYSWRNDR	=	$6B
SYSWUPD		=	$6D
SYSBSEED	=	$6F

; These tuned with a C128 wedge connected to a 1702 monitor 
; with L/C cable and controls tweaked:
;	COLOR at max
;	BRIGHTNESS at 9 o'clock
;	CONTRAST 3 at o'clock
SYSCLTBL
		!byte	.BLACK		; zap color
		!byte	.DARKGRAY	; zap transition color
		!byte	.GREEN		; fade color
		!byte	.LTGREEN	; body color
		!byte	.LTGREEN	; lead transition color
		!byte	.WHITE		; lead color

; Table of possible trace lengths.  Table must be 2^n > 1 size.
SYSLENS		!byte	8, 8, 16, 17, 18, 19, 20, 21
.LENSLEN	=	(* - SYSLENS)	; Length of table
SYSLENSM	=	(.LENSLEN - 1)	; Mask for constrconstraining selection.


SYSINIT
		; Initialize random number generator (noise channel).
		lda	#$ff		; maximum frequency value
		sta	.SIDFQ3LO	; voice 3 frequency low byte
		sta	.SIDFQ3HI	; voice 3 frequency high byte
		lda	#$80		; noise waveform, gate bit off
		sta	.SIDCTRL3	; voice 3 control register

		; Set VIC-II background colors.
		lda	#.BORDER
		sta	.VIC2BRDR
		lda	#.BACKGND
		sta	.VIC2BGND

		; Set VIC-II character definitions address.
		sei
		lda	.VM1
		and	#%11110000
		ora	#(>SYSCHRAM / 4)
		sta	.VM1		
		cli
		rts


SYSSTOP		
		bit	.STKEY
		bpl	+		; bit-7 = 0 when RUN/STOP pressed
		rts
+		jmp	(.RSTVEC)	; Reset


SYSRANDB
		lda	.SIDNOISE
		rts


;----------------------------------
; END OF SYSTEM DEFINITIONS
;
		+BLAUNCH_ALIGN	; Necessary for BASIC implementation.
.SYSEND

} ; !zone C128

; Continue to main code.
!src "main.inc"
