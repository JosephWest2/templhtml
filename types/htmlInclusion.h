#pragma once
#include <stddef.h>

typedef struct htmlEntry htmlEntry;

typedef struct htmlInclusion {
    htmlEntry *entry;
    size_t iStart;
    size_t iEnd;
} htmlInclusion;
