MEMORY
{
    rom (rx) : ORIGIN = 0x0, LENGTH = 512K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

INCLUDE src/linker-scripts/common_xip.ld
