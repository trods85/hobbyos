ENTRY(_start)

SECTIONS {
	. = 0x100000;

	.text : {
		*(.text.entry)
		*(.text*)
	}

	.rodata : { *(.rodata*) }
	.data : { *(.data*) }
	.bss : ALIGN(16) {
		__bss_start = .;
		*(COMMON)*(.bss*)
		__bss_end = .;
		}
	.stack (NOLOAD): ALIGN(16) {
		. += 0x10000; /*64 KiB*/
		__stack_top = .;
	}
}
