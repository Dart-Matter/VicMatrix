
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
; Generates the PETSCII decimal representation of the specified value into the
; current program counter location, and advances the program counter.
;
!macro basic_num .v {
    !set .r = 5 ; Max allowed digits for our use case.
    !set .d = 0 ; Current digit.
    !for .i,1,.r {
	!set .d = (.d & $f0) | (.v/(10^(.r-1))*$11) ; Compute digit and leading zeros skipped.
	!set .v = (.v-(.d & $0f)*(10^(.r-1))) * 10  ; Shift source digit out.
	!if (.d & $f0) { !byte '0' + (.d & $0f) }   ; Conditional: Skipped leading zeros.
    }
}


;----------------------------------
; This is used to adjust the address being called by the SYS statement so
; that it can be stored in the REM statement.
;
!macro BLAUNCH_ALIGN {
    !if <*=0 | >*=0 { nop }
}


;----------------------------------
; Generates a BASIC launcher program to SYS the specified address.
;
!macro BLAUNCH .addr {
  !ifdef .addr {
    ; The VIC-20 can't handle 0's embedded in BASIC statements.
    ; We must correct this in order to put the 6502 JMP in a REM statement.
    !if <.addr=0 | >.addr=0 { !error "Bad BLAUNCH target alignment.  Precede target with +BLAUNCH_ALIGN." }
  }
		; REM statement
-		!word	+		; Link to next statement.
		!word	-		; Unique line number is current address.
		!byte	$8f		; REM statement token.
.entry		jmp	.addr		; Embedded jump to main.
		!byte	0		; End of REM statement.

		; SYS statement
+		
-		!word	+		; Link to end of BASIC launcher program.
		!word	-		; Unique line number is current address.
		!byte	$9e		; SYS statement token.
		+basic_num .entry	; Place entry address into statement as text.
		!byte	0		; End of SYS statement.

		; End of program
+		!word	0		; End of BASIC luancher program.
}
