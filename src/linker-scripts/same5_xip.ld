MEMORY
{
    rom (rx) : ORIGIN = 0x00000000, LENGTH = 1024K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
    backup (rw) : ORIGIN = 0x47000000, LENGTH = 8K
}

INCLUDE src/linker-scripts/common_xip.ld
