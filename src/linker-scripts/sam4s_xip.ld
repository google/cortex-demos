MEMORY
{
    rom (rx) : ORIGIN = 0x0, LENGTH = 1024K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 160K
}

INCLUDE src/linker-scripts/common_xip.ld
