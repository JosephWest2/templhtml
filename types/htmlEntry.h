#pragma once
#include "htmlInclusion.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct htmlEntry {
    char path[128];
    char fileName[64];
    htmlInclusion *inclusions;
    size_t inclusionCount;
    size_t inclusionMax;
    bool built;
} htmlEntry;

void HtmlEntry_Init(htmlEntry *entry, char *path, char *fileName);

void HtmlEntry_AddHtmlInclusion(htmlEntry *entryToAddTo, size_t iStart, size_t iEnd, htmlEntry *referencedEntry);

htmlInclusion *HtmlEntry_IterateInclusions(htmlEntry *entry, size_t *i);
