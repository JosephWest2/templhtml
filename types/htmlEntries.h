#pragma once
#include "htmlEntry.h"
#include <stddef.h>

typedef struct htmlEntries {
    htmlEntry *entries;
    size_t entryCount;
    size_t maxEntries;
} htmlEntries;

void HtmlEntries_Init(htmlEntries *entries);

htmlEntry *GetAssociatedHtmlEntry(htmlEntries *entries, char *fileName);

void HtmlEntries_AddEntry(htmlEntries *entries, char *path, char *fileName);

htmlEntry *HtmlEntries_Iterate(htmlEntries *entries, size_t *i);
