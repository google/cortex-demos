# demOS (demo-OS) {#mainpage}

This project is a microcontroller operating system that focuses heavily on testability.

Essentially, this is a research project, trying to figure out how far the concept can be taken,
without using real hardware. The idea of being testable is what drives most of the design choices.

The OS is actually just a driver model, it uses [FreeRTOS](https://freertos.org) for scheduling
and IPC.

** This is not an officially supported Google product **

## Design for Test

The project focuses on tests from the very beginning. This means that even very simple code,
even the smallest driver must be tested and in true TDD tradition, it must be possible to write
test first.

If you look at the firmware that is running on a typical ARM32 microcontroller,
from the perspective of the firmware (software), the hardware is simply a
set of registers, accessible through memory IO operations, plus interrupts.

Let's say you have a simple GPIO module. To set particular line high, you set the value of a
corresponding bit in a 32 bit register.

```c

const uint32_t gpio_reg_addr = 0x50000508;
const unsigned gpio_line = 8;

/* Setting the line high */
*((volatile uint32_t*)gpio_reg_addr) = (1 << gpio_line);

```

Unfortunately, if we want to test this code off the hardware, i.e. on regular desktop
computer, that memory write operation is kind of hard to mock.

The solution this project uses is very simple and is summed up in
[memio.h](src/memio.h). All memory IO operations are defined as a function-like macros,
for example:

```c
#define raw_write32(addr, value)     *((volatile uint32_t*)(addr)) = (value)
```

When certain preprocessor option is given during compilation (`TEST_MEMIO`), this function-like
macro becomes a real function:

```
void raw_write32(uint32_t addr, uint32_t value);
```

For tests this function simply delegates to a method of a global object:

```c++
mock::Memory g_memory;

extern "C" void raw_write32(uint32_t addr, uint32_t value) {
    g_memory.write32(addr, value);
}
```

This object is essentially a `std::map`, that maps from virtual, microcontroller's memory
address to the value in that memory location.

It has more features, of course, but this is the basic idea.
