.include "xosera.inc"

.export xoboing

vram_base_a		= $0000
vram_base_b		= $A000
vram_base_blank	= $B800
vram_base_ball	= $BC00
tile_base_b		= $C000
size			= 19200

WIDTH_A = 320


WIDTH_B = 640
HEIGHT_B = 480

COLS_PER_WORD_A = 8

WIDTH_WORDS_A = WIDTH_A / COLS_PER_WORD_A

TILE_WIDTH_B = 8
TILE_HEIGHT_B = 8
WIDTH_WORDS_B = WIDTH_B / TILE_WIDTH_B
HEIGHT_WORDS_B = HEIGHT_B / TILE_HEIGHT_B

BALL_BITMAP_WIDTH =  256
BALL_BITMAP_HEIGHT =  256

BALL_TILES_WIDTH = (BALL_BITMAP_WIDTH / TILE_WIDTH_B)
BALL_TILES_HEIGHT = (BALL_BITMAP_WIDTH / TILE_HEIGHT_B)

.ZEROPAGE

src:				.res 2
srcend:				.res 2
dest:				.res 2

tile:		.res 2

incval:				.res 1

.segment "VAR"

lastbyte:		.res 1		; last byte read
destlen:		.res 2		; number of bytes written

xr_wrptr:		.res 2		; write function pointer

line_len:		.res 1
row_base:		.res 2

palette_base:	.res 2

.segment "XOBOING"

bg_bitmap:
.incbin "../build/bg_real.rle-toolkit"

ball_tiles:
.incbin "../build/tiles.rle-toolkit"

palettes:
.incbin "../build/palettes.rle-toolkit"

copperlist:
.incbin "../build/copperlist.rle-toolkit"

xoboing:
	xosera_wr_extended XR_VID_CTRL, $0000

	; setup playfield A
	xosera_wr_extended XR_PA_GFX_CTRL, (XR_GFX_CTRL_BM | XR_GFX_CTRL_BPP_1 | XR_GFX_CTRL_HX_2 | XR_GFX_CTRL_VX_2) ; bitmap, 1 bit/pixel, 2xwide, 2xhigh
	xosera_wr_extended XR_PA_DISP_ADDR, vram_base_a

	; setup playfield B
	xosera_wr_extended XR_PB_GFX_CTRL, (1<<4) ; tiled, 4 bit/pixel
	xosera_wr_extended XR_PB_TILE_CTRL, (tile_base_b | XR_TILE_CTRL_MAP_IN_VRAM | XR_TILE_CTRL_DEF_IN_VRAM | XR_TILE_CTRL_HEIGHT_8) ; tile memory at $C000, tiledata in VRAM, tiles 8 bit high
	xosera_wr_extended XR_PB_DISP_ADDR, vram_base_b

	; PA Colours:
	xosera_wr_extended XR_COLOR_A_ADDR + 0, $0BBB ; Grey
	xosera_wr_extended XR_COLOR_A_ADDR + 1, $0B0B ; Purple

	xosera_wr16 XR_PA_LINE_LEN, WIDTH_WORDS_A
	xosera_wr16 XR_PB_LINE_LEN, WIDTH_WORDS_B

	; PB Colours:
	ldx #14 ; palette index
	lda #<XR_COLOR_B_ADDR
	sta palette_base
	lda #>XR_COLOR_B_ADDR
	sta palette_base + 1

	; write background
	xosera_wr16 XM_WR_ADDR, vram_base_a ; VRAM write ptr
	xosera_wr16 XM_WR_INCR, 1 ; VRAM write stepwidth

	lda #<bg_bitmap
	sta src

	lda #>bg_bitmap
	sta src+1

	lda #<xr_wr_lo_only
	sta xr_wrptr
	lda #>xr_wr_lo_only
	sta xr_wrptr+1

	lda #$01
	xosera_sta_hi XM_DATA

	jsr rle_unpack

	; write ball tiles
	xosera_wr16 XM_WR_ADDR, tile_base_b ; VRAM write ptr

	lda #<ball_tiles
	sta src

	lda #>ball_tiles
	sta src+1

	lda #<xr_wr_hi
	sta xr_wrptr
	lda #>xr_wr_hi
	sta xr_wrptr+1

	jsr rle_unpack

	; write color palettes
	xosera_wr16 XM_XR_ADDR, XR_COLOR_B_ADDR ; extended area write ptr

	lda #<palettes
	sta src

	lda #>palettes
	sta src+1

	lda #<xr_wr_xr_hi
	sta xr_wrptr
	lda #>xr_wr_xr_hi
	sta xr_wrptr+1

	jsr rle_unpack

	; write copperlist
	xosera_wr16 XM_XR_ADDR, XR_COPPER_ADDR ; extended area write ptr

	lda #<copperlist
	sta src

	lda #>copperlist
	sta src+1

	lda #<xr_wr_xr_hi
	sta xr_wrptr
	lda #>xr_wr_xr_hi
	sta xr_wrptr+1

	jsr rle_unpack

	; Load PB tilemap
	; Two extra rows needed for fine scrolling, one for the bottom row, and one for the bottom right tile

	ldx #WIDTH_WORDS_B
	ldy #HEIGHT_WORDS_B + 2
	lda #WIDTH_WORDS_B
	sta line_len
	lda #<vram_base_b
	sta row_base
	lda #>vram_base_b
	sta row_base + 1
	lda #0
	sta tile
	sta tile + 1
	sta incval
	jsr vram_fill_tiled

	; Load blank tilemap

	ldx #BALL_TILES_WIDTH
	ldy #BALL_TILES_HEIGHT
	lda #BALL_TILES_WIDTH
	sta line_len
	lda #<vram_base_blank
	sta row_base
	lda #>vram_base_blank
	sta row_base + 1
	lda #0
	sta tile
	sta tile + 1
	sta incval
	jsr vram_fill_tiled

	; Load ball tilemap
	ldx #BALL_TILES_WIDTH
	ldy #BALL_TILES_HEIGHT
	lda #BALL_TILES_WIDTH
	sta line_len
	lda #<vram_base_ball
	sta row_base
	lda #>vram_base_ball
	sta row_base + 1
	lda #0
	sta tile
	sta tile + 1
	lda #1
	sta incval
	jsr vram_fill_tiled

.if 0
	; Write ball to screen
	ldx #BALL_TILES_WIDTH
	ldy #BALL_TILES_HEIGHT
	lda #WIDTH_WORDS_B
	sta line_len
	lda #<vram_base_b
	sta row_base
	lda #>vram_base_b
	sta row_base + 1
	lda #0
	sta tile
	sta tile + 1
	lda #1
	sta incval
	jsr vram_fill_tiled
.endif

.if 0
; false color palette to show sectors
	xosera_wr_extended (XR_COLOR_B_ADDR + $2), $F000 ;  // Black
	xosera_wr_extended (XR_COLOR_B_ADDR + $3), $FFFF ;  // White
	xosera_wr_extended (XR_COLOR_B_ADDR + $4), $FF00 ;  // Red
	xosera_wr_extended (XR_COLOR_B_ADDR + $5), $FF70 ;  // Orange
	xosera_wr_extended (XR_COLOR_B_ADDR + $6), $FFF0 ;  // Yellow
	xosera_wr_extended (XR_COLOR_B_ADDR + $7), $F7F0 ;  // Spring Green
	xosera_wr_extended (XR_COLOR_B_ADDR + $8), $F0F0 ;  // Green
	xosera_wr_extended (XR_COLOR_B_ADDR + $9), $F0F7 ;  // Turquoise
	xosera_wr_extended (XR_COLOR_B_ADDR + $A), $F0FF ;  // Cyan
	xosera_wr_extended (XR_COLOR_B_ADDR + $B), $F07F ;  // Ocean
	xosera_wr_extended (XR_COLOR_B_ADDR + $C), $F00F ;  // Blue
	xosera_wr_extended (XR_COLOR_B_ADDR + $D), $F70F ;  // Violet
	xosera_wr_extended (XR_COLOR_B_ADDR + $E), $FF0F ;  // Magenta
	xosera_wr_extended (XR_COLOR_B_ADDR + $F), $FF07 ;  // Raspberry
.endif

.if 0
; rotate palettes synchronized to video
@colors:
	xosera_wr_extended XR_PB_GFX_CTRL, ($0000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($1000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($2000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($3000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($4000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($5000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($5000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($7000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($8000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($9000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($A000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($B000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($C000 | (1<<4))
	jsr vswait
	xosera_wr_extended XR_PB_GFX_CTRL, ($D000 | (1<<4))
	jsr vswait

	jmp @colors

vswait:
	ldx #10 ; wait for 10 frames
@loop:
	xosera_wr16 XM_XR_ADDR, XR_SCANLINE
	xosera_lda_hi XM_XR_DATA
	and #1<<7
	beq @loop
	dex
	bne @loop
	rts

.endif

.if 1
	xosera_wr_extended (XR_COPPER_ADDR + $0002 << 1 | $1), (1<<4)

	xosera_wr_extended (XR_COPPER_ADDR | $01), (vram_base_b + WIDTH_WORDS_B * 10)
	xosera_wr_extended (XR_COPPER_ADDR | $11), 0

	xosera_wr_extended XR_COPP_CTRL, (1<<15)
.endif

	rts


; write lo byte only
xr_wr_lo_only:
	xosera_sta_lo XM_DATA
	jmp rle_store_cont

; write lo byte and prepare writing hi byte in next cycle
xr_wr_lo:
	xosera_sta_lo XM_DATA
	pha
	lda #<xr_wr_hi
	sta xr_wrptr
	lda #>xr_wr_hi
	sta xr_wrptr+1
	pla
	jmp rle_store_cont

; write hi byte and prepare writing lo byte in next cycle
xr_wr_hi:
	xosera_sta_hi XM_DATA
	pha
	lda #<xr_wr_lo
	sta xr_wrptr
	lda #>xr_wr_lo
	sta xr_wrptr+1
	pla
	jmp rle_store_cont

; write extended area lo byte and prepare writing hi byte in next cycle
xr_wr_xr_lo:
	xosera_sta_lo XM_XR_DATA
	pha
	lda #<xr_wr_xr_hi
	sta xr_wrptr
	lda #>xr_wr_xr_hi
	sta xr_wrptr+1
	pla
	jmp rle_store_cont

; write extended area hi byte and prepare writing lo byte in next cycle
xr_wr_xr_hi:
	xosera_sta_hi XM_XR_DATA
	pha
	lda #<xr_wr_xr_lo
	sta xr_wrptr
	lda #>xr_wr_xr_lo
	sta xr_wrptr+1
	pla
	jmp rle_store_cont

; rle implementation is based on code Copyright (c) 2004, Per Olofsson
; read a byte and increment source pointer
rle_read:
	lda (src),y
	inc src
	bne :+
	inc src + 1
:	rts

; write a byte and increment destination pointer
rle_store:
	jmp (xr_wrptr)
rle_store_cont:
	inc dest
	bne :+
	inc dest + 1
:	inc destlen
	bne :+
	inc destlen + 1
:	rts

; unpack a run length encoded stream
rle_unpack:
	ldy #0
	sty destlen		; reset byte counter
	sty destlen + 1
	jsr rle_read		; read the first byte
	sta lastbyte		; save as last byte
	jsr rle_store		; store
@unpack:
	jsr rle_read		; read next byte
	cmp lastbyte		; same as last one?
	beq @rle		; yes, unpack
	sta lastbyte		; save as last byte
	jsr rle_store		; store
	jmp @unpack		; next
@rle:
	jsr rle_read		; read byte count
	tax
	beq @end		; 0 = end of stream
	lda lastbyte
@read:
	jsr rle_store		; store X bytes
	dex
	bne @read
	beq @unpack		; next
@end:
	rts

; row_base:		first row
; line_len:		video memory line length
; tile:			tile to fill
; incval:		add this to tile every round
; X register:	width
; Y register:	height
vram_fill_tiled:
@rows:
	xosera_wrp16 XM_WR_ADDR, row_base
	lda row_base
	clc
	adc line_len
	sta row_base
	lda #0
	adc row_base + 1
	sta row_base + 1

	txa
	pha

@cols:
	xosera_wrp16 XM_DATA, tile
	lda incval
	clc
	adc tile
	sta tile
	lda #0
	adc tile + 1
	sta tile + 1
	dex
	bne @cols

	pla
	tax

	dey
	bne @rows

	rts