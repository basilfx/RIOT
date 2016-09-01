#define LOAD_ADDRESS 0x10000BC8

	.syntax unified
	.global	cortex_vectors
cortex_vectors:
start:
	.org  0x00000000

	.word 0x96969999
	.word 0xFC66CC3F
	.word 0x03CC33C0
	.word 0x6231DCE5
	.word end - load_address
	.word LOAD_ADDRESS                             // hardcoded load address
	.word 0xFFFF0000
	.word 0xFFFFFFFF
load_address:                                          // from here it's loaded into memory, first word is jump address
	.word LOAD_ADDRESS + (init - load_address) + 1 // add 1 for thumb mode
	.word ADDRESS
	.word ADDRESS
	.word ADDRESS
	.word ADDRESS
	.word 0x88167923                               // some kind of checksum for load address???
	.word 0xFFFFFFFF
	.word 0xFFFFFFFF
init:
	cpsid f
	mov r2, #ADDRESS
	ldr r3, [r2, #0]
	mov sp, r3
	ldr r3, [r2, #4]
	bx r3
	.org (load_address - start) + (ADDRESS - LOAD_ADDRESS)
address:
	.incbin FILENAME
end:
