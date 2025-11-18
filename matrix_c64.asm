
;------------------------------------------------------------------------------
;
; Copyright 1983, 2023 Rodney Rushing
;
;
; This file is part of MATRIX for Commodore Color Computers.
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

!src "cbm_launcher.inc"

!zone C64 {

;----------------------------------
; BASIC ENTRY POINT
;
							* =	$801
		+BLAUNCH .SYSEND	; Generate BASIC launcher program that
					; jumps to the end of this zone.


;----------------------------------
; SYSTEM CONSTANTS
;
.BLACK		=	0
.DARKGRAY	=	11
.GRAY		=	12
.GREEN		=	5
.LTGREEN	=	13
.WHITE		=	1
.RED		=	2

.BORDER		=	.BLACK
.BACKGND	=	.BLACK
.STKEY		=	$91
.RSTVEC		=	$fffc

; Random number generation
.SIDNOISE	=	$d41b
.SIDFQ3LO	=	$d40e
.SIDFQ3HI	=	$d40f
.SIDCTRL3	=	$d412

; VIC-II Configuration
.VIC2MCTL	=	$d018
.CHRADMSK	=	%00001110
.VIC2BRDR	=	$d020
.VIC2BGND	=	$d021


;----------------------------------
; GLOBAL SYSTEM ABSTRACTION (see main.asm for descriptions.)
;

CFGSLOW		=	0
CFGLAYERS	=	2
CFGFADE		=	0

SYSROWS		=	25		; default
SYSCOLS		=	40		; default
SYSVXRAM		=	$0400		; default
SYSCLRAM	=	$D800		; default
SYSCHRAM	=	$2000

T1		=	$61
T2		=	$63
T3		=	$65
SYSWRAND	=	$67
SYSWRNDR	=	$69
SYSWUPD		=	$6B
SYSBSEED	=	$6D

; These tuned with a C64 breadbin connected to a 1702 monitor 
; with L/C cable and controls centered.
SYSCLTBL
		!byte	.BLACK		; zap color
		!byte	.DARKGRAY	; zap transition color
		!byte	.GREEN		; fade color
		!byte	.LTGREEN	; body color
		!byte	.LTGREEN	; lead transition color
		!byte	.WHITE		; lead color

; Table of possible trace lengths.  Table must be 2^n > 1 size.
SYSLENS		!byte	3, 8, 9, 10, 11, 19, 20, 21
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
		lda	.VIC2MCTL
		and	#!.CHRADMSK
		ora	#((SYSCHRAM >> 10) & .CHRADMSK)
		sta	.VIC2MCTL
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
		+BLAUNCH_ALIGN		; Necessary for BASIC implementation.
.SYSEND

} ; !zone C64

; Continue to main code.
!src "main.inc"
