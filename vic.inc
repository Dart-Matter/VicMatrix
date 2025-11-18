
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

vic_cr0         = $9000
vic_cr0_ilace   = %10000000 ; Enable interlace
vic_cr0_orgx    = %01111111 ; Horizontal position[6:0]

vic_cr1         = $9001     ; Vertical position[7:0]
vic_cr1_orgy    = %11111111 ; Vertical position[7:0]

vic_cr2         = $9002
vic_cr2_scrnlo  = %10000000 ; Screen base[9]
vic_cr2_cols    = %01111111 ; Column count[6:0]

vic_cr3         = $9003
vic_cr3_rastlo  = %10000000 ; Raster[0], read-only
vic_cr3_rows    = %01111110 ; Row count[5:0]
vic_cr3_dbl     = %00000001 ; Enable double height characters

vic_cr4         = $9004     ; Raster[8:1], read-only

vic_cr5         = $9005
vic_cr5_scrnhi  = %11110000 ; Screen base[13:10]
vic_cr5_chars   = %00001111 ; Character base[13:10]
vic_chars_rom_upper         = $00>>2
vic_chars_rom_reverse_upper = $04>>2
vic_chars_rom_lower         = $08>>2
vic_chars_rom_reverse_lower = $0c>>2
vic_chars_ram_0400          = $24>>2
vic_chars_ram_0800          = $28>>2
vic_chars_ram_0c00          = $2c>>2
vic_chars_ram_1000          = $30>>2
vic_chars_ram_1400          = $34>>2
vic_chars_ram_1800          = $38>>2
vic_chars_ram_1c00          = $3c>>2

; To generate base address from vic_chars use:
;   (($80 * (1 - (vic_chars >> 3))) | ((vic_chars & $07)<<2))<<8

vic_cr6         = $9006     ; Light pen horizontal[7:0]

vic_cr7         = $9007     ; Light pen vertical[7:0]

vic_cr8         = $9008     ; Pot X[7:0]

vic_cr9         = $9009     ; Pot Y[7:0]

vic_cra         = $900a     ; Low voice
vic_cra_lowen   = %10000000 ; Enable voice
vic_cra_lowfreq = %01111111 ; Frequency[6:0]

vic_crb         = $900b     ; Mid voice
vic_crb_miden   = %10000000 ; Enable voice
vic_crb_midfreq = %01111111 ; Frequency[6:0]

vic_crc         = $900c     ; High voice
vic_crc_hien    = %10000000 ; Enable voice
vic_crc_hifreq  = %01111111 ; Frequency[6:0]

vic_crd         = $900d     ; Noise
vic_crd_nyzen   = %10000000 ; Enable
vic_crd_nyzfreq = %01111111 ; Frequency[6:0]

vic_cre_hien    = $900e
vic_cre_aux     = %11110000 ; Auxiliary color[3:0]
vic_cre_vol     = %00001111 ; Level[3:0]

vic_crf         = $900f
vic_crf_bgnd    = %11110000 ; Background color[3:0]
vic_crf_norvs   = %00001000 ; Disable reverse mode
vic_crf_border  = %00000111 ; Border color[2:0]

color_ram_base_lo = $9400
color_ram_base_hi = $9600

color_black     = 0
color2_black    = color_black
color_white     = 1
color2_white    = color_white
color_red       = 2
color2_red      = color_red
color_cyan      = 3
color2_seafoam  = color_cyan
color_purple    = 4
color2_magenta  = color_purple
color_green     = 5
color2_teal     = color_green
color_blue      = 6
color2_purple   = color_blue
color_yellow    = 7
color2_yellow   = color_yellow
color_orange    = 8
color2_orange   = color_orange
color_ltorange  = 9
color2_peach    = color_ltorange
color_pink      = 10
color2_pink     = color_pink
color_ltcyan    = 11
color2_mint     = color_ltcyan
color_ltpurple  = 12
color2_mauve    = color_ltpurple
color_ltgreen   = 13
color2_green    = color_ltgreen
color_ltblue    = 14
color2_sky      = color_ltblue
color_ltyellow  = 15
color_mustard   = color_ltyellow
