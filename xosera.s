.export xosera_init

xosera_base = $8000

xm_xr_addr = $0
xm_xr_data = $1
xm_xr_rd_incr = $2
xm_xr_rd_addr = $3
xm_xr_wr_incr = $4
xm_xr_wr_addr = $5
xm_data = $6

pa_gfx_ctrl = $10

.define xosera_reg(addr) (xosera_base + addr * 2)

; write register
.macro xosera_wr16 addr, data16
	lda #>data16
	sta xosera_base + addr * 2
	lda #<data16
	sta xosera_base + addr * 2 + 1
.endmacro

; store A to xosera register, lowbyte only
; Xosera uses the last written highbyte in this case
.macro xosera_sta_lo addr
	sta xosera_base + addr * 2 + 1
.endmacro

; write extended register
.macro xosera_wr_extended addr, data16
	xosera_wr16 xm_xr_addr, addr
	xosera_wr16 xm_xr_data, data16
.endmacro

.segment "XOSERA"

xosera_init:
	xosera_wr_extended pa_gfx_ctrl, 0 ; enable textmode
	xosera_wr16 xm_xr_wr_addr, 0 ; VRAM write ptr
	xosera_wr16 xm_xr_rd_addr, 0 ; VRAM read ptr
	xosera_wr16 xm_xr_wr_incr, 1 ; VRAM write stepwidth
	xosera_wr16 xm_xr_rd_incr, 1 ; VRAM read stepwidth

	xosera_wr16 xm_data, $f100 | 'A'
	xosera_wr16 xm_data, $f200 | 'B'
	xosera_wr16 xm_data, $f300 | 'C'

	ldy #0
l1:
	lda mytext,y
	beq exit
	xosera_sta_lo xm_data
	iny
	bne l1

exit:
	rts

mytext:
	.byte "Welcome to zeromips.org"
	.byte $00
