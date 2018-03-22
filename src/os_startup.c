extern unsigned _data_loadaddr, _data, _edata, _bss, _ebss;
typedef void (*funcp_t) (void);
extern funcp_t __preinit_array_start, __preinit_array_end;
extern funcp_t __init_array_start, __init_array_end;
extern funcp_t __fini_array_start, __fini_array_end;

void main(void);

void __attribute__ ((weak, naked, section(".reset"))) os_reset_handler(void)
{
	volatile unsigned *src, *dest;
	funcp_t *fp;

	for (src = &_data_loadaddr, dest = &_data;
		dest < &_edata;
		src++, dest++) {
		*dest = *src;
	}

	for (dest = &_bss; dest < &_ebss; ) {
		*dest++ = 0;
	}

	/* Constructors. */
	for (fp = &__preinit_array_start; fp < &__preinit_array_end; fp++) {
		(*fp)();
	}
	for (fp = &__init_array_start; fp < &__init_array_end; fp++) {
		(*fp)();
	}

	/* Call the application's entry point. */
	main();

	/* Destructors. */
	for (fp = &__fini_array_start; fp < &__fini_array_end; fp++) {
		(*fp)();
	}
}
