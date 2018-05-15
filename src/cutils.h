#pragma once

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))

#define ABSDIFF(a, b) \
    ({\
        __typeof__(a) _a = (a);\
        __typeof__(b) _b = (b);\
        _a > _b ? (_a - _b) : (_b - _a);})
