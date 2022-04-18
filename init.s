.import xosera_init
.import xoboing
.export nmi, puls, start
.import __STACKSTART__ ; Linker generated

.include "zeropage.inc"

.segment "INIT"
nmi:
	jmp nmi

puls:
	jmp puls

start:

	sei ; disable the interrupts, as fast as possible - they are disabled in case of HW reset,
	    ; but this routine can be also called manually

	cld ; required for all CPUs - due to possibility of manual call

	ldx #$FF
	txs

	; initialize C runtime stack
	lda #<__STACKSTART__
	ldx #>__STACKSTART__
	sta sp
	stx sp+1

	jsr xosera_init
	jsr xoboing

loop:
	jmp loop
