
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

;----------------------------------
; SYSTEM FILE FOR C16 and PLUS4
;

!src "cbm_launcher.inc"

!zone C16 {

;----------------------------------
; BASIC ENTRY POINT
;
				* =	$1001
		+BLAUNCH .SYSEND	; Generate BASIC launcher program that
					; jumps to the end of this zone.


;----------------------------------
; SYSTEM CONSTANTS
;
.LUM0		=	$00
.LUM1		=	$10
.LUM2		=	$20
.LUM3		=	$30
.LUM4		=	$40
.LUM5		=	$50
.LUM6		=	$60
.LUM7		=	$70
.BLACK		=	0
.GRAYS		=	1
.REDS		=	2
.CYANS		=	3
.MAGENTAS	=	4
.GREENS		=	5
.PURPLES	=	6
.OLIVES		=	7
.ORANGES	=	8
.BROWNS		=	9
.PINES		=	10
.PINKS		=	11
.MINTS		=	12
.BLUES		=	13
.VIOLETS	=	14
.LIMES		=	15
.WHITE		=	.GRAYS + .LUM7
.BORDER		=	.BLACK
.BACKGND	=	.BLACK
.STKEY		=	$91
.CHROUT		=	$ffd2
.RSTVEC		=	$fffc

.TEDCOLR4	=	$ff19
.TEDBKGND	=	$ff15
.TEDMSEL	=	$ff12
.ROMRAMSL	=	4
.TEDCHRAD	=	$ff13
.CHRADMSK	=	%11111100


;----------------------------------
; GLOBAL SYSTEM ABSTRACTION (see main.asm for descriptions.)
;

CFGSLOW		=	0
CFGLAYERS	=	2
CFGFADE		=	0

SYSROWS		=	25		; C16 default
SYSCOLS		=	40		; C16 default
SYSVXRAM		=	$0C00		; C16 default
SYSCLRAM	=	$0800		; C16 default
SYSCHRAM	=	$2000

T1		=	$61
T2		=	$63
T3		=	$65
SYSWRAND	=	$67
SYSWRNDR	=	$69
SYSWUPD		=	$6B
SYSBSEED	=	$6D

; These tuned on a television quality composite CRT monitor.
SYSCLTBL	!byte	.GRAYS + .LUM1	; zap color
		!byte	.GREENS + .LUM0	; zap transition color
		!byte	.GREENS + .LUM3	; fade color
		!byte	.GREENS + .LUM4	; body color
		!byte	.GREENS + .LUM6	; lead transition color
		!byte	.WHITE		; lead color

; Table of possible trace lengths.  Table must be 2^n > 1 size.
SYSLENS		!byte	3, 8, 9, 10, 11, 19, 20, 21
.LENSLEN	=	(* - SYSLENS)	; Length of table
SYSLENSM	=	(.LENSLEN - 1)	; Mask for constrconstraining selection.


SYSINIT
		; Background colors.
		lda	#.BORDER
		sta	.TEDCOLR4
		lda	#.BACKGND
		sta	.TEDBKGND

		; Enable custom characters
		lda	.TEDMSEL 
		and	#!.ROMRAMSL
		sta	.TEDMSEL

		; Set character definitions address.
		sei
		lda	.TEDCHRAD
		and	#<!.CHRADMSK
		ora	#(>SYSCHRAM & .CHRADMSK)
		sta	.TEDCHRAD
		cli
		rts
 
 
SYSSTOP		
		bit	.STKEY
		bpl	+		; bit-7 = 0 when RUN/STOP pressed
		rts
+
		; If the key is pressed when a reset occurs, the kernel 
		; automatically enters the machine language monitor.
		; We'll wait for the key to be released.
-		bit	.STKEY		; Read again.
		bpl	-		; Still pressed, loop.
		sei			; Disable interrupts.
		jmp	(.RSTVEC)	; Reset.


; SYSRANDB
;
; Use default random number generator.
!src "sysrandb.inc"

.SYSEND

} ; !zone C16

; Continue to main code.
!src "main.inc"
