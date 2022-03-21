; ZeroMips jumptable

.import puls, start, nmi

	.segment "VECTORS"
	.word nmi        ;program defineable
	.word start      ;initialization code
	.word puls       ;interrupt handler
