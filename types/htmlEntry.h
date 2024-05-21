#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct htmlEntry {
    char path[128];
    char fileName[64];
    size_t id;
    bool built;
} htmlEntry;

void HtmlEntry_Create(htmlEntry *entry, char *path, char *fileName);
