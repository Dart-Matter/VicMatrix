
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

;
; VARIATION: by White Flame
;
; See https://www.codebase64.org/doku.php?id=base:small_fast_8-bit_prng
;
; No variation of xor is needed for our application.  The random numbers are
; distributed across multiple functions which inherently translate into
; some entropy. If necessary here are the other xors that can be used:
;
; xorvar !byte $1d, $2b, $2d, $4d, $5f, $63, $65, $69, $71, $78, $8d, $a9, $c3, $cf, $e7, $f5
;
; This is fast.
;

!zone SYSRAND {

SYSRANDB
		lda	SYSBSEED
		beq	.xor
		asl
		beq	.noxor
		bcc	.noxor
.xor		eor	#$1d
.noxor		sta	SYSBSEED
		rts
}
