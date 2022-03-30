.include "xosera.inc"

.export xosera_init

.segment "XOSERA"

xosera_init:
	xosera_wr_extended XR_PA_GFX_CTRL, 0 ; enable textmode
	xosera_wr16 XM_WR_ADDR, 0 ; VRAM write ptr
	xosera_wr16 XM_RD_ADDR, 0 ; VRAM read ptr
	xosera_wr16 XM_WR_INCR, 1 ; VRAM write stepwidth
	xosera_wr16 XM_RD_INCR, 1 ; VRAM read stepwidth

	xosera_wr16 XM_DATA, $f100 | 'A'
	xosera_wr16 XM_DATA, $f200 | 'B'
	xosera_wr16 XM_DATA, $f300 | 'C'

	ldy #0
l1:
	lda mytext,y
	beq exit
	xosera_sta_lo XM_DATA
	iny
	bne l1

exit:
	rts

mytext:
	.byte "Welcome to zeromips.org"
	.byte $00
