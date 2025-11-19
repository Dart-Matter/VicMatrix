
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

;------------------------------------------------------------------------------
;
; For ACME Assembler
;
; This file should be included by a target system definition assembler file
; (matrix_xxxx.asm).
;
; The overall theory of operation is filling the screen with characters and
; using only the color attributes to control their appearance.  Each screen
; column will contain one or more logical and independently moving rain traces,
; and use the attributes to visualize them.
;
; Each trace is logically divided into segments that can be displayed 
; differently using the color attributes; this allows a more appealing fading 
; effect.  The segment's parameters were chosen purely by eyeballing the results
; on a monitor.
;

!zone MATRIX
		jmp	.MAIN


;----------------------------------
; CONFIGURATION VALUES
;
; Each system file defines necessary functions and data specific to the
; respective system.  
;



;----------------------------------
; SYSTEM ABSTRACTION
;
; Each system file defines necessary functions and data specific to the
; respective system:
;
; CFGSLOW	A numeric value that initializes the STALL loop counter which 
;		controls the overall speed.
;
;   		Valid values: 0 (fastest) .. 65535 (slowest)
;
; CFGLAYERS	A numeric value that specifies how many traces to generate on
;		each column.  This makes the screen look more sophisticated at
;		the expense of speed.  Of the systems tested so far, 1 and 2
;		are the only practical values.
;
; CFGFADE	Forces the fade visual effect for 1 layer configurations.
;		This effect is automatically enabaled with more than one layer
;		because it's the same amount of work.  The resulting 
;		visualization is achieved through the color table values.
;
;   		Valid values: 0 = effect not forced, 1 = effect forced
;
; SYSCLTBL	Address of a byte table containing color values for each 
;		visual attribute type.
;
; SYSVXRAM	The memory address of the video matrix (no pun intended).
;
; SYSCLRAM	The memory address of color RAM.
;
; SYSCHRAM	The memory address to write character definitions.
;
; SYSROWS	The video height in characters.
;
; SYSCOLS	The video width in characters.
;
; SYSLEN	Table of possible trace lengths and...
; SYSLENSM	Mask that defines valid length table index bits.
;		The table size must be 2^n > 1.  2^n is necessary to make it
;		trivial (quick) to convert a random number into an index.
;
; SYSINIT	A function to do all system initialization.  
;		No registers are assumed preserved.
;
; SYSSTOP	A function to detect an escape key and exit the program.
;		This function should not return if the key has been pressed.
;
; SYSRANDB	A function to get a random byte value.  A default implementation
;		is provided in file sysrandb.inc.
;		Returns: A = random value in the range [0..255].
;		Preserves: X, Y, temporaries
;
; Dedicated zero-page.  Choose addresses that won't crash the system.
;
; T1, T2, T3	Zero-page temporary words (2-bytes) used for varying purposes.
;		All users must assume that subroutines will not preserve these
;		unless explicitly stated.
;
; SYSBSEED	(byte for default random number generator)
;
; SYSWRAND	(word used for RANDOM function local variables)
;
; SYSWRNDR	(word used for RENDER function local variables)
;
; SYSWUPD	(word used for UDPATE function local variables)
;

;----------------------------------
; GLOBAL DATA
;

; Visual attributes
;
ZAP		=	0		; Background.
DARK		=	1		; Transition to background.
DIM		=	2		; Fading part of body.
NORMAL		=	3		; Normal part of body.
LIGHT		=	4		; Transition from leading character.
BRIGHT		=	5		; Leading character.
MUTATE		=	%10000000	; Character mutate flag.

; Tables
;
TRACES		=	SYSCOLS * CFGLAYERS
!if CFGLAYERS < 1 { !error "Minimum layers is 1." }
!if TRACES > 255 { !error "Too many traces declared." }


; Table of possible descent rates.
;
; This is implemented as a table so that odds of a particular rate being 
; selected can be manually tuned.  If only one rate is desired just specify 
; all the same value.
;
RATES		!byte	1, 2, 4, 8	; Number of entries must be 2^n > 1
RATESLEN	=	(* - RATES)	; Length of this table.
RATEMASK	=	(RATESLEN - 1)	; Mask for selecting from random value.

; Import the character set definition.
CHRSET
!src "kat.inc"
CHSETLEN 	=	(* - CHRSET)	; Size of character set definition.
NUMCHARS 	=	CHSETLEN / 8	; Number of characters defined.


;----------------------------------
; HANDY MACROS
;

; Load an immediate 16-bit value lo/hi into A/X registers respectively
!macro laxi .a {
		lda	#<.a
		ldx	#>.a
}

; Store A/X at address lo/hi bytes respectively
!macro stax .a {
		sta	.a
		stx	.a+1
}

; Unsigned branch if register value is below operand.
!macro bib .a {
		bcc	.a
}

; Unsigned branch if register value is below or equals operand.
!macro bbe .a {
		beq	.a
		bcc	.a
}

; Unsigned branch if register value is above or equals operand.
!macro bae .a {
		bcs	.a
}

; Unsigned branch if register value is above operand.
!macro bia .a {
		beq	+
		bcs	.a
+
}

!macro avertb {
		!byte 	$24
}

!macro avertw {
		!byte	$2c
}

;----------------------------------
; MAIN LOOP
;
.MAIN
		jsr	MTRXINIT	; Initialize the matrix.
-		jsr	UPDATE		; Draw and advance all traces one time.
		jsr	SYSSTOP		; Detect stop key.
		jsr	STALL		; Delay a bit per configuration.
		jmp	-		; Loop endlessly.


;----------------------------------
; INITIALIZE
;
; Initializes all the important state.
;
; Returns: nothing.
;
; Preserves: nothing.
;
!zone MTRXINIT
MTRXINIT
		jsr	SYSINIT		; Initialize system.

		;
		; Clear screen.
		;
.cptr		=	T1		; Alias for readability; color pointer
.vptr		=	T2		; Alias for readability; video pointer

		+laxi	SYSCLRAM	; Get color RAM address.
		+stax	.cptr		; Store it in zero page.
		+laxi	SYSVXRAM		; Get video matrix address.
		+stax	.vptr		; Store it in zero page.
		ldx	#>(SYSCOLS * SYSROWS) ; The number of full pages.
		ldy	#0		; Initialize the running write offset.
.i1		lda 	SYSCLTBL + ZAP	; The color used for clearing.
		sta 	(.cptr),Y	; Clear a color RAM byte.
		; Place a random character.
		lda 	#NUMCHARS	; Number of characters in set.
		jsr 	RANDOM		; Select a random one.
		sta 	(.vptr),Y	; Write it to the video RAM.
		iny			; Next write offset.
		bne	.i1		; Repeat until wrap back to 0.
		inc 	.cptr + 1	; Next color page.
		inc 	.vptr + 1	; Next video page.
		dex			; Any pages left?
		bne	.i1		; Yes, do another full page.
		; A partial page remains.
		ldy 	#<(SYSCOLS * SYSROWS) - 1 ; Number of characters to erase.
.i2		lda 	SYSCLTBL + ZAP	; The color used for clearing.
		sta 	(.cptr),Y	; Clear a color RAM byte.
		; Place a random character.
		lda 	#NUMCHARS	; Number of characters in set.
		jsr 	RANDOM		; Select a random one.
		sta 	(.vptr),Y	; Write it to the video RAM.
		dey			; Next write offset.
		cpy	#-1		; Did we decrement past 0 to -1?
		bne	.i2		; No, do another.

		;
		; Copy character definitions.
		;
.count		=	T1
.src		=	T2
.dst		= 	T3

		+laxi	CHSETLEN	; Load character definition size
		+stax	.count		; ...into zero page copy count.
		+laxi	CHRSET		; Load source char definition address
		+stax	.src		; ...into zero page source pointer.
		+laxi	SYSCHRAM	; Load target char definition address
		+stax	.dst		; ...into zero page target pointer.
		ldy	#0		; Start copying at offset 0.
.i3		lda	(.src),Y	; Copy one byte from source
		sta	(.dst),Y	; ...to target.
		iny			; Increment copy offset.
		bne	.i4		; Same page until offset wraps to 0.
		inc	.src + 1	; Next source page.
		inc	.dst + 1	; Next target page.
.i4		dec	.count		; Decrement copy count.
 		bne	.i3		; Continue until 0 bytes remaining.
 		dec	.count + 1	; Decrement count MSB.
 		bpl	.i3		; Repeat until page -1 is reached.

		;
		; Initialize screen and color row address tables.
		;
		ldy	#>SYSVXRAM	; Y will track screen page.
		lda	#0		; A will track page offset.
		tax			; X will track table index.
.mi1		; Write pages for one row.
		sta	ROWLTBL,X	; Store LSB of row address.
		pha			; Backup page offset.
		tya			; Copy page byte so we can write it.
		sta	ROWHVTBL,X	; Write page of screen row to table.
		clc			; Start addition.
		adc	#>(SYSCLRAM - SYSVXRAM) ; Add screen/color page difference.
		sta	ROWHCTBL,X	; Write page of color row to table.
		pla			; Restore page offset.
		; Advance to next row.
		clc			; Start addition.
		adc	#SYSCOLS	; Add stride between rows to LSB.
		bcc	+		; No carry means same page.
		iny			; Next page.
+		inx			; Next table entry.
		cpx	#SYSROWS	; Index is past last table entry?
		bcc	.mi1		; No, do another.

		;
		; Initialize each trace.
		;
		ldx	#0		; Trace index.
		ldy	#0		; Screen column.
.nxtcol		lda	#CFGLAYERS	; Reset layer count.
		sta	T3		; Store it in counter variable.
.nxttrc		jsr	TRANDIZE	; Initialize trace with random parameters.
		inx			; Next trace.
		cpx	#TRACES		; All traces initialized?
		bne	+		; Yes.
		rts			; No, done.
+		dec	T3		; Tick off a layer for this column.
		bne	.nxttrc		; Do another layer on this column.
		iny			; Next column.
		jmp	.nxtcol		; (This always branches.)


;----------------------------------
; STALL EXECUTION
;
; Spends time doing nothing useful.
;
; Returns: CF = 0 for callers that would like to unconditionally branch
;          upon return.
;
; Preserves: A, temporaries
;
!zone STALL
STALL
		ldx	#(<CFGSLOW)+1	; Load LSB+1 to make it a byte count.
		ldy	#(>CFGSLOW)+1	; Load MSB+1 to make it a page count.
-		dex			; Decrement byte count.
		bne	-		; If bytes remaining, loop.
		dey			; Decerement page count.
		bne	-		; If pages remaining, loop.
		rts


;----------------------------------
; RANDOM NUMBER GENERATOR
;
; This generates a number in a specified range size.  This is only necessary 
; for non- n^2 ranges < 255, otherwise calling SYSRANDB and masking bits is 
; faster.
;
;	A = range size less than 256.
;
; Returns: A = random number [0..(range-1)]
;
; Preserves: X, Y, temporaries
;
!zone RANDOM
RANDOM

.range		=	SYSWRAND
.mask		=	SYSWRAND + 1

		sta	.range		; Save range.
		sta	.mask		; Save mutable range for mask calculation.
		; Create mask that encompasses all bits of the range.
		lda	#0		; Clear mask.
-		sec			; Shift one bit into mask
		rol			; ...LSb.
		lsr	.mask		; Shift range to detect max 1 bit accounted for.
		bne	-		; There are more 1 bits, create another mask bit.
		sta	.mask		; Mask calculated; save it.
		jsr	SYSRANDB	; Get a random byte value.
		and	.mask		; Mask it to a value close to range.
		cmp	.range		; How does it compare to range?
		+bib	.end		; If it's below range, we're done.
		sbc	.range		; It's too big; shrink it.
.end		rts


;----------------------------------
; INITIALIZE A RAIN TRACE
;
; Variants are provided which all set a random length, but control 
; leading position.
;
;	X = trace index
;	Y = screen column to assign
;
; Returns: nothing.
;
; Preserves: X, Y
;
!zone TINIT

; Initialize to random length and position.
TRANDIZE	sec
		bcs	TINIT
; Initializes to random length at top of screen.
TRESET		clc
		; Intentionally fall-through.
; Initialize to random length, and CF selects random position.
TINIT		sty	T2		; Backup Y arg.
		; Set trace column.
		tya			; Copy column for write to indexed.
		sta	COLTBL,X	; Write to the trace columns table.
		; Set trace leading position.
		lda	#0		; Top of screen.
		bcc	+		; Not random leading.
		lda	#SYSROWS	; Range for random row.
		jsr	RANDOM		; Get a random row.
+		sta	LEADTBL,X	; Write to the trace leading table.
		; Random trace length.
		pha			; Backup leading position.
		jsr 	SYSRANDB	; Get random byte value.
		and 	#SYSLENSM	; Convert to length table index.
		tay			; Index the length table.
		lda 	SYSLENS,Y	; Get the selected length.
		sta 	LENTBL,X	; Write to the trace length table.
		pla			; Restore leading position.
		sec			; Starting subtraction.
		sbc	LENTBL,X	; Sub trace length to get zap position.
		sta	ZAPTBL,X	; Write to trace zap position table.
		; Random trace descent rate.
		jsr	SYSRANDB	; Get random byte value.
		and	#RATEMASK	; Convert to rate table index.
		tay			; Index the rates table.
		lda	RATES,Y		; Get the selected rate.
		sta	RATETBL,X	; Write to trace rate table.
		sta	DECAYTBL,X	; Reset trace decay.
		ldy	T2		; Restore Y arg.
		rts


;----------------------------------
; DESCEND A TRACE AND REGERENATE A NEW ONE IF IT GOES OFF SCREEN.
;
;   X = trace index
;
; Returns: nothing
;
; Preserves: X, temporaries
;
!zone DESCEND
DESCEND		lda	ZAPTBL,X	; Get zap position.
		bmi	+		; Above top of screen; descend.
		cmp	#SYSROWS - 1	; Below bottom of screen?
		+bib	+		; No, still on screen; descend.
		; Set new trace parameters.
		ldy	COLTBL,X	; Keep column for this trace.
		bpl	TRESET		; Initialize new trace at screen top.
		; Descend trace by one character
+		inc	LEADTBL,X	; Move leading down.
+		inc	ZAPTBL,X	; Move zap down.
		lda	RATETBL,X	; Use trace descent rate to
		sta	DECAYTBL,X	; ...reset decay counter.
		rts


;----------------------------------
; WRITE AN ATTRIBUTE TO THE SCREEN
;
; Writes color attributes and/or mutates an existing screen character.
;
;   X = trace index
;   Y = screen row
;   A = attributes
;
; Returns: nothing
;
; Preserves: X, Y
;
!zone PAINT
PAINT

.attr		=	T3
.row		=	T3 + 1
.ptr		=	T2

		; Ignore if position is off screen.
		cpy	#0		; Test row.
		bmi	.end		; Negative rows are above screen top.
		cpy	#SYSROWS	; Compare row to row beyond lowest.
		+bae	.end		; It's below last screen row.
		; Initialize
		sta	.attr		; Save attribute.
		sty	.row		; Save Y.
!if (CFGLAYERS > 1) {
		; Add clipping for opaque elements.
		cmp	#ZAP		; If this is a zap, don't clip other
		beq	+		; ...layers, i.e. make it transparent.
		sta	CLIPTBL,Y	; Any non-zero value will do as clip flag.
+
}
		lda	ROWLTBL,Y	; Convert row to row address LSB.
		sta	.ptr		; Write it to shared pointer LSB.
		; Mutate attribute?
		lda	.attr		; Get attribute.
		and	#MUTATE		; Mutate flag set?
		beq	+		; No character mutation requested.
		; Change a screen character.
		lda	ROWHVTBL,Y	; Convert row to video row MSB.
		sta	.ptr + 1	; Write to shared pointer MSB.
		jsr	SYSRANDB	; Get a random mutation value.
		and	#%00001111	; Mask mutation bits.
		ldy	COLTBL,X	; Get the trace column.
		eor	(.ptr),Y	; Get and mutate character.
		sta	(.ptr),Y	; Write character to video screen.
		; Color attribute?
+		lda	.attr		; Recall attribute.
		cmp	#MUTATE		; Is it character-only?
		beq	+		; Yes, nothing else to do.
		; Apply color.
		ldy	.row		; Recall attribute row.
		lda	ROWHCTBL,Y	; Convert row to color row MSB.
		sta	.ptr + 1	; Write shared pointer MSB (LSB is same).
		lda	.attr		; Recall attribute.
		and	#<!MUTATE	; Isolate color attribute.
		tay			; Use as index.
		lda	SYSCLTBL,Y	; Get associated attribute color.
		ldy	COLTBL,X	; Get screen column.
		sta	(.ptr),Y	; Write color to color RAM.
+		ldy	.row		; Restore Y.
.end		rts


;----------------------------------
; DRAW A RAIN TRACE
;
; Enumerates the characters of a trace and has them drawn if not clipped.
; There are "fade" and "simple" variants that trade off speed and visual
; effect respectively.
;
;   X = trace index
;
; Returns: nothing
;
; Preserves: X
;
!zone RENDER
RENDER

;
; NOTE: Only one of the following methods is ever compiled into the program.
;

;
; METHOD #1: If there are multiple layers or fancy colors, draw full 
; traces every time.  (Back layers may be clipped.)
;
; We do this by painting the two end characters and work toward the
; middle until they cross or are off screen.  This section starts with
; the second to end characters; the ends have already been painted as 
; common to either method.
;
!if (CFGLAYERS > 1) or (CFGFADE > 0) {

!macro .clip .a {
	!if (CFGLAYERS > 1) {
		lda	CLIPTBL,Y	; Is it clipped?
		bne	.a		; Yes, branch.
	}
}

.upper		=	SYSWRNDR
.lower		=	SYSWRNDR + 1

		ldy	LEADTBL,X	; Get leading position into call parameter.
		sty	.lower		; Save it for tracking the lower row.
		cpy	#SYSROWS	; Is it below screen?
		+bae	+		; Yes, don't paint it.
		+.clip	+		; Skip painting if clipped.
		lda	#BRIGHT		; Leading attribute.
		jsr	PAINT		; Paint it.

+		ldy	ZAPTBL,X	; Get zap position into call parameter.
		sty	.upper		; Save it for tracking the upper row.
		bmi	+		; Negative values are above screen, don't paint it.
		+.clip	+		; Skip painting if clipped.
		lda	#ZAP		; Zap attribute.
		jsr	PAINT		; Paint it.
+
 		dec	.lower		; Next lower row.
		bmi	+		; Lower is above top of screen.
 		ldy	.lower		; Load into call parameter.
 		cpy	#SYSROWS	; Is it below screen?
 		+bae	+		; Yes, don't paint it.
		+.clip	+		; Skip painting if clipped.
		lda	#LIGHT		; Lower transition attribute.
		jsr	PAINT		; Paint it.

+		inc	.upper		; Next upper row.
		ldy	.upper		; Load into call parameter.
		bmi	.body		; Negative values are off screen, don't paint it.
		+.clip	.body		; Skip painting if clipped.
		lda	#DARK		; Upper transition attribute.
		jsr	PAINT		; Paint it.

.body		inc	.upper		; Next upper row.
		ldy	.upper		; Load into call parameter.
		bmi	+		; Negative values are off screen, don't paint it.
		cpy	.lower		; Overlapped lower?
		beq	.end
		+.clip	+		; Skip painting if clipped.
		lda	#DIM		; Upper body attribute.
		jsr	PAINT		; Paint it.

+		ldy	.lower		; Next lower row.
		dey
		sty	.lower
		bmi	.end		; Lower is above screen, done.
		bit	.upper		; If upper is above top of screen
		bmi	+		; ...then lower has not crossed it.
		cpy	.upper		; Has lower crossed upper?
		+bbe	.end		; Yes, done.
+		cpy	#SYSROWS	; Is lower off screen?
 		+bae	+		; Yes, don't paint it.
		+.clip	+		; Skip painting if clipped.
		lda	#NORMAL		; Lower body attribute.
		jsr	PAINT		; Paint it.
+		jmp	.body		; Do more.
.end


; METHOD #2: If there is only one trace per column and the fancy colors 
; are not forced on, then the only thing remaining is to repaint the old
; leading character as a body segment.
} else {

		ldy	LEADTBL,X	; Get leading position into call parameter.
		lda	#BRIGHT		; Leading attribute.
		jsr	PAINT		; Paint it.

		dey			; Move up one row from leading.
		lda	#NORMAL		; A body attribute.
		jsr	PAINT		; Paint it.

		ldy	ZAPTBL,X	; Get zap position into call parameter.
		lda	#ZAP		; Zap attribute.
		jsr	PAINT		; Paint it.
}
;
; Maybe mutate a screen character.
;
		; Randomize how often this occurs.
		; This was chosen by eyeballing.
		jsr	SYSRANDB	; Get a random byte value.
		and	#3		; More bits decreases chances.
		bne	+		; If not all 0 skip mutation.
		; Mutate
		lda	LENTBL,X	; Get trace length.
		sec			; Start subtracting.
		sbc	#2		; Candidates exclude leading and zap.
		jsr	RANDOM		; Get random row offset.
		sec			; Skip zap row.
		adc	ZAPTBL,X	; Add offset to zap.
		tay			; Set as paint row parameter.
		lda	#MUTATE		; Set attribute.
		jsr	PAINT		; Paint it.
+		rts


;----------------------------------
; ADVANCE STATE AND DRAW ONE SCREEN OF RAIN
;
; Preserves: nothing
;
!zone UPDATE
UPDATE

.column		=	SYSWUPD		; Track columns drawn.
.layer		=	SYSWUPD + 1	; Track layers drawn.

		ldx	#0		; Running trace index.
		lda	#SYSCOLS	; Reset column count.
		sta	.column		; Store column loop counter.
.nextcolm	lda	#CFGLAYERS	; Reset layer count.
		sta	.layer		; Store later loop counter.
!if (CFGLAYERS > 1) {
		lda 	#0		; Clear clip flags.
		ldy 	#SYSROWS	; Number of clip flags to clear.
-		sta 	CLIPTBL-1,Y	; Clear a flag.
		dey			; Next flag index.
		bne	-		; Loop until all flags cleared.
}
.nextlayr	dec	DECAYTBL,X	; Decay.  Time to advance this trace?
		bne	+		; No.
		jsr	DESCEND		; Advance.
+		jsr	RENDER		; Draw the trace.
		inx			; Next trace.
		dec	.layer		; Reduce layer count.
		bne	.nextlayr	; Layers remain, next layer.
		dec	.column		; Reduce column count.
		bne	.nextcolm	; Columns remain, next column.
		rts
ENDCODE

; BSS

COLTBL		; Columns assigned to each trace.
*		=	* + TRACES

ZAPTBL		; Trailing position of each trace.
* 		= 	* + TRACES

LEADTBL		; Leading position of each trace.
*		=	* + TRACES

RATETBL		; Descent rate of each trace.
*		=	* + TRACES

DECAYTBL	; Decay counter of each trace.
*		=	* + TRACES

LENTBL		; Length of each trace.
*		=	* + TRACES

ROWLTBL		; Low bytes of each row address.
*		=	* + SYSROWS

ROWHVTBL	; High bytes of each video matrix row address.
*		=	* + SYSROWS

ROWHCTBL	; High bytes of each color RAM row address.
*		=	* + SYSROWS

!if CFGLAYERS > 1 {
CLIPTBL		; For clipping overlapping traces.
*		=	* + SYSROWS
}

ENDBSS
