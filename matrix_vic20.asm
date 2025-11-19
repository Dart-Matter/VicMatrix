
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

!zone VIC20 {

!src "vic.inc"
!src "cbm_launcher.inc"

;----------------------------------
; BASIC ENTRY POINT
;
!ifdef MEM3K {
		* = $401
		+BLAUNCH .SYSEND	; Generate BASIC launcher program.
} else ifdef MEM8K {
		* = $1201
		+BLAUNCH .SYSEND
		!skip 	256		; Push everything out 1 page; video matrix encroaches here.
} else {
		* = $1001
		+BLAUNCH .SYSEND
}


;----------------------------------
; SYSTEM CONSTANTS
;
!ifdef PAL {
.VPOS		=	28		; Eye-balled.
.HPOS		=	9		; Eye-balled.
} else {
.VPOS		=	15		; Eye-balled.
.HPOS		=	1		; Eye-balled.
}
.BORDER		=	color_black
.BACKGND	=	color_black
.VICVBASE	=	SYSVXRAM | $2000 ; This is the address VIC sees.
!ifdef MEM8K {
.CHARBASE	=	vic_chars_ram_1c00 ; Select custom chars at $1C00.
} else {
.CHARBASE	=	vic_chars_ram_1800 ; Select custom chars at $1800.
}
.STKEY		=	$91
.RSTVEC		=	$fffc


;----------------------------------
; GLOBAL SYSTEM ABSTRACTION (see main.asm for descriptions.)
;
CFGSLOW		=	100
CFGLAYERS	=	2
CFGFADE		=	0

SYSROWS		=	29		; Eye-balled NTSC.
SYSCOLS		=	25		; Eye-balled NTSC.
!ifdef MEM8K {
SYSVXRAM		=	$1000		; What the CPU sees.
} else {
SYSVXRAM		=	$1c00		; What the CPU sees.
}
SYSCHRAM	=	(($80 * (1 - (.CHARBASE >> 3))) | ((.CHARBASE & $07)<<2))<<8
SYSCLRAM	=	color_ram_base_lo + (SYSVXRAM & $0200); What CPU sees.
					; (Color RAM address is dictated by 
					; which half of a 1K "page" the video 
					; RAM is located on.)
T1		=	$61
T2		=	$63
T3		=	$65
SYSWRAND	=	$67
SYSWRNDR	=	$69
SYSWUPD		=	$6B
SYSBSEED	=	$6D

; These tuned on a television quality composite CRT monitor.
SYSCLTBL	!byte	color_black	; zap color
		!byte	color_green	; zap transition color
		!byte	color_green	; fade color
		!byte	color_green	; body color
		!byte	color_green	; lead transition color
		!byte	color_white	; lead color

; Table of possible trace lengths.  Table must be 2^n > 1 size.
SYSLENS		!byte	3, 8, 9, 10, 11, 19, 20, 21
.LENSLEN	=	(* - SYSLENS)	; Length of table
SYSLENSM	=	(.LENSLEN - 1)	 ; Mask for constrconstraining selection.


SYSINIT
		; Horizontal position
		lda	vic_cr0
		and	#NOT vic_cr0_orgx
		ora	#.HPOS
		sta	vic_cr0
		; Vertical position
		lda	#.VPOS
		sta	vic_cr1
		; Screen base, char base, and columns
		lda	#((>.VICVBASE << 6) & vic_cr2_scrnlo) + SYSCOLS
		sta	vic_cr2
		lda	#((>.VICVBASE<<2) & vic_cr5_scrnhi) + .CHARBASE
		sta	vic_cr5
		; Rows / single height
		lda	#SYSROWS << 1
		sta	vic_cr3
		; Black background
		lda	#((.BACKGND << 4) & vic_crf_bgnd) + (.BORDER & vic_crf_border) + vic_crf_norvs
		sta	vic_crf
		cli
		rts


SYSSTOP		
		lda	.STKEY
		cmp	#%11111110	; Only STOP pressed.
		beq	+
		rts
		sei
+		jmp	(.RSTVEC)


; SYSRANDB

!if 1 {

; Use default random number generator.
!src "sysrandb.inc"

} else {

; For reference: Using VIC kernel RND function -- very slow.

.VICRAND	=	$e094		; Result is returned in VIC-20's Floating-Point #1.
.FP1		=	$61		; Floating-Point #1 mantissa bytes address (4 bytes)

SYSRANDB
		txa
		pha
		tya
		pha
		jsr	.VICRAND	; Generate random float.
		pla
		tay
		pla
		tax
		lda	.FP1 + 1	; Get a result mantissa byte.
		rts
}

.SYSEND

} ; !zone VIC20

; Continue to main code.
!src "main.inc"
