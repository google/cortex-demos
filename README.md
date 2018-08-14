# demOS (demo-OS)

This project is a microcontroller operating system that focuses heavily on testability.

Essentially, this is a research project, trying to figure out how far the concept can be taken,
without using real hardware. The idea of being testable is what drives most of the design choices.

The OS is actually just a driver model, it uses [FreeRTOS](https://freertos.org) for scheduling
and IPC.

