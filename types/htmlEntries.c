#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "htmlEntries.h"
#include "htmlEntry.h"

static void _Grow(htmlEntries * entries) {
    entries->bufferLength *= 2;
    entries->entries = realloc(entries->entries, entries->bufferLength * sizeof(htmlEntry));
}

void HtmlEntries_Init(htmlEntries * entries) {
    entries->entryCount = 0;
    entries->bufferLength = 20;
    entries->entries = malloc(sizeof(htmlEntry) * entries->bufferLength);
}

void HtmlEntries_Cleanup(htmlEntries * entries) {
    free(entries->entries);
}

htmlEntry *GetAssociatedHtmlEntry(htmlEntries *entries, char *fileName) {
    for (size_t i = 0; i < entries->entryCount; i++) {
        if (strcmp(entries->entries[i].fileName, fileName) == 0) {
            return &entries->entries[i];
        }
    }
    return NULL;
}

void HtmlEntries_AddEntry(htmlEntries *entries, char *path, char *fileName) {
    if (entries->entryCount >= entries->bufferLength) {
        _Grow(entries);
    }
    HtmlEntry_Create(&entries->entries[entries->entryCount], path, fileName);
    entries->entryCount++;
}

htmlEntry *HtmlEntries_Iterate(htmlEntries *entries, size_t *i) {
    if (*i < entries->entryCount && *i < entries->bufferLength) {
        (*i)++;
        return &entries->entries[*i - 1];
    }
    return NULL;
}
