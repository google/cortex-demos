#include <cstdint>
#include <cstddef>

extern "C" void* pvPortMalloc(size_t xWantedSize);
extern "C" void* vPortFree(void *pv);

void operator delete(void* ptr) {
    vPortFree(ptr);
}

void operator delete(void* ptr, unsigned) {
    vPortFree(ptr);
}

void* operator new  (size_t count) {
    return pvPortMalloc(count);
}

void* operator new[](size_t count) {
    return pvPortMalloc(count);
}
