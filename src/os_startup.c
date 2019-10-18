/*******************************************************************************
    Copyright 2018 Google LLC

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        https://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*******************************************************************************/

extern unsigned _data_loadaddr, _data, _edata, _bss, _ebss;
typedef void (*funcp_t)(void);
extern funcp_t __preinit_array_start, __preinit_array_end;
extern funcp_t __init_array_start, __init_array_end;
extern funcp_t __fini_array_start, __fini_array_end;

void main(void);

void
#ifndef CHIP_NATIVETEST
__attribute__((weak, naked, section(".reset")))
#endif
os_reset_handler(void) {
    volatile unsigned* src, *dest;
    funcp_t* fp;

    for (src = &_data_loadaddr, dest = &_data;
         dest < &_edata;
         src++, dest++) {
        *dest = *src;
    }

    for (dest = &_bss; dest < &_ebss;) {
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
