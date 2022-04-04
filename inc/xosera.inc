XOSERA_BASE = $8000

; Xosera XR Memory Regions (size in 16-bit words)
XR_COLOR_ADDR   = $8000 ; (R/W) 0x8000-0x81FF 2 x A & B color lookup memory
XR_COLOR_SIZE   = $0200 ;                      2 x 256 x 16-bit words  (0xARGB)
XR_COLOR_A_ADDR = $8000 ; (R/W) 0x8000-0x80FF A 256 entry color lookup memory
XR_COLOR_A_SIZE = $0100 ;                      256 x 16-bit words (0xARGB)
XR_COLOR_B_ADDR = $8100 ; (R/W) 0x8100-0x81FF B 256 entry color lookup memory
XR_COLOR_B_SIZE = $0100 ;                      256 x 16-bit words (0xARGB)
XR_TILE_ADDR    = $A000 ; (R/W) 0xA000-0xB3FF tile glyph/tile map memory
XR_TILE_SIZE    = $1400 ;                      5120 x 16-bit tile glyph/tile map memory
XR_COPPER_ADDR  = $C000 ; (R/W) 0xC000-0xC7FF copper program memory (32-bit instructions)
XR_COPPER_SIZE  = $0800 ;                      2048 x 16-bit copper program memory addresses
XR_UNUSED_ADDR  = $E000 ; (-/-) 0xE000-0xFFFF unused

; Xosera Main Registers (XM Registers, directly CPU accessable)
XM_XR_ADDR   = $00 ; (R /W+) XR register number/address for XM_XR_DATA read/write access
XM_XR_DATA   = $01 ; (R /W+) read/write XR register/memory at XM_XR_ADDR (XM_XR_ADDR incr. on write)
XM_RD_INCR   = $02 ; (R /W ) increment value for XM_RD_ADDR read from XM_DATA/XM_DATA_2
XM_RD_ADDR   = $03 ; (R /W+) VRAM address for reading from VRAM when XM_DATA/XM_DATA_2 is read
XM_WR_INCR   = $04 ; (R /W ) increment value for XM_WR_ADDR on write to XM_DATA/XM_DATA_2
XM_WR_ADDR   = $05 ; (R /W ) VRAM address for writing to VRAM when XM_DATA/XM_DATA_2 is written
XM_DATA      = $06 ; (R+/W+) read/write VRAM word at XM_RD_ADDR/XM_WR_ADDR (and add XM_RD_INCR/XM_WR_INCR)
XM_DATA_2    = $07 ; (R+/W+) 2nd XM_DATA(to allow for 32-bit read/write access)
XM_SYS_CTRL  = $08 ; (R /W+) busy status, FPGA reconfig, interrupt status/control, write masking
XM_TIMER     = $09 ; (R /W+) read 1/10th millisecond timer, write clear interrupt signal
XM_LFSR      = $0A ; (RO)    LFSR pseudo-random number // TODO: keep this?
XM_UNUSED_B  = $0B ; (R /W ) unused direct register 0xB // TODO: slated for XR_DATA_2 after reorg
XM_RW_INCR   = $0C ; (R /W ) XM_RW_ADDR increment value on read/write of XM_RW_DATA/XM_RW_DATA_2
XM_RW_ADDR   = $0D ; (R /W+) read/write address for VRAM access from XM_RW_DATA/XM_RW_DATA_2
XM_RW_DATA   = $0E ; (R+/W+) read/write VRAM word at XM_RW_ADDR (and add XM_RW_INCR)
XM_RW_DATA_2 = $0F ; (R+/W+) 2nd XM_RW_DATA(to allow for 32-bit read/write access)

; XR Extended Register / Region (accessed via XM_XR_ADDR and XM_XR_DATA)

; Video Config and Copper XR Registers
XR_VID_CTRL  = $00 ; (R /W) display control and border color index
XR_COPP_CTRL = $01 ; (R /W) display synchronized coprocessor control
XR_UNUSED_02 = $02 ; (R /W) // TODO: to be refactored
XR_UNUSED_03 = $03 ; (R /W) // TODO: to be refactored
XR_UNUSED_04 = $04 ; (R /W) // TODO: to be refactored
XR_UNUSED_05 = $05 ; (R /W) // TODO: to be refactored
XR_VID_LEFT  = $06 ; (R /W) left edge of active display window (typically 0)
XR_VID_RIGHT = $07 ; (R /W) right edge +1 of active display window (typically 640 or 848)
XR_SCANLINE  = $08 ; (RO  ) [15] in V blank, [14] in H blank [10:0] V scanline
XR_UNUSED_09 = $09 ; (RO  )
XR_VERSION   = $0A ; (RO  ) Xosera optional feature bits [15:8] and version code [7:0] [TODO]
XR_GITHASH_H = $0B ; (RO  ) [15:0] high 16-bits of 32-bit Git hash build identifier
XR_GITHASH_L = $0C ; (RO  ) [15:0] low 16-bits of 32-bit Git hash build identifier
XR_VID_HSIZE = $0D ; (RO  ) native pixel width of monitor mode (e.g. 640/848)
XR_VID_VSIZE = $0E ; (RO  ) native pixel height of monitor mode (e.g. 480)
XR_VID_VFREQ = $0F ; (RO  ) update frequency of monitor mode in BCD 1/100th Hz (0x5997 = 59.97 Hz)

; Playfield A Control XR Registers
XR_PA_GFX_CTRL  = $10 ; (R /W) playfield A graphics control
XR_PA_TILE_CTRL = $11 ; (R /W) playfield A tile control
XR_PA_DISP_ADDR = $12 ; (R /W) playfield A display VRAM start address
XR_PA_LINE_LEN  = $13 ; (R /W) playfield A display line width in words
XR_PA_HV_SCROLL = $14 ; (R /W) playfield A horizontal and vertical fine scroll
XR_PA_LINE_ADDR = $15 ; (- /W) playfield A scanline start address (loaded at start of line)
XR_PA_HV_FSCALE = $16 ; (R /W) playfield A horizontal and vertical fractional scale

; Playfield B Control XR Registers
XR_PB_GFX_CTRL  = $18 ; (R /W) playfield B graphics control
XR_PB_TILE_CTRL = $19 ; (R /W) playfield B tile control
XR_PB_DISP_ADDR = $1A ; (R /W) playfield B display VRAM start address
XR_PB_LINE_LEN  = $1B ; (R /W) playfield B display line width in words
XR_PB_HV_SCROLL = $1C ; (R /W) playfield B horizontal and vertical fine scroll
XR_PB_LINE_ADDR = $1D ; (- /W) playfield B scanline start address (loaded at start of line)
XR_PB_HV_FSCALE = $1E ; (R /W) playfield B horizontal and vertical fractional scale
XR_PB_UNUSED_1F = $1F ;

XR_GFX_CTRL_BLANK	= 1<<7
XR_GFX_CTRL_BM		= 1<<6
XR_GFX_CTRL_BPP_1	= 0<<4
XR_GFX_CTRL_BPP_4	= 1<<4
XR_GFX_CTRL_BPP_8	= 2<<4
XR_GFX_CTRL_HX_1	= 0<<2
XR_GFX_CTRL_HX_2	= 1<<2
XR_GFX_CTRL_HX_3	= 2<<2
XR_GFX_CTRL_HX_4	= 3<<2
XR_GFX_CTRL_VX_1	= 0<<0
XR_GFX_CTRL_VX_2	= 1<<0
XR_GFX_CTRL_VX_3	= 2<<0
XR_GFX_CTRL_VX_4	= 3<<0

XR_TILE_CTRL_MAP_IN_VRAM	= 0<<9
XR_TILE_CTRL_MAP_IN_TILEMEM	= 1<<9
XR_TILE_CTRL_DEF_IN_VRAM	= 1<<8
XR_TILE_CTRL_DEF_IN_TILEMEM	= 0<<8
XR_TILE_CTRL_HEIGHT_1		= 0
XR_TILE_CTRL_HEIGHT_2		= 1
XR_TILE_CTRL_HEIGHT_3		= 2
XR_TILE_CTRL_HEIGHT_4		= 3
XR_TILE_CTRL_HEIGHT_5		= 4
XR_TILE_CTRL_HEIGHT_6		= 5
XR_TILE_CTRL_HEIGHT_7		= 6
XR_TILE_CTRL_HEIGHT_8		= 7
XR_TILE_CTRL_HEIGHT_9		= 8
XR_TILE_CTRL_HEIGHT_10		= 9
XR_TILE_CTRL_HEIGHT_11		= 10
XR_TILE_CTRL_HEIGHT_12		= 11
XR_TILE_CTRL_HEIGHT_13		= 12
XR_TILE_CTRL_HEIGHT_14		= 13
XR_TILE_CTRL_HEIGHT_15		= 14

; write 16 bit constant to register
.macro xosera_wr16 addr, data16
	lda #>data16
	sta XOSERA_BASE + addr * 2
	lda #<data16
	sta XOSERA_BASE + addr * 2 + 1
.endmacro

; write 16 bit variable to register
.macro xosera_wrp16 addr, pdata16
	lda pdata16 + 1
	sta XOSERA_BASE + addr * 2
	lda pdata16
	sta XOSERA_BASE + addr * 2 + 1
.endmacro

; store A to xosera register, lowbyte only
; Xosera uses the last written highbyte in this case
.macro xosera_sta_lo addr
	sta XOSERA_BASE + addr * 2 + 1
.endmacro

.macro xosera_sta_hi addr
	sta XOSERA_BASE + addr * 2
.endmacro

.macro xosera_lda_lo addr
	lda XOSERA_BASE + addr * 2 + 1
.endmacro

.macro xosera_lda_hi addr
	lda XOSERA_BASE + addr * 2
.endmacro

; write extended register
.macro xosera_wr_extended addr, data16
	xosera_wr16 XM_XR_ADDR, addr
	xosera_wr16 XM_XR_DATA, data16
.endmacro
