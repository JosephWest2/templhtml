#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "htmlEntries.h"
#include "htmlEntry.h"
#include "htmlEntry.c"
#include "../main.h"

typedef struct htmlEntries {
    htmlEntry *entries;
    size_t entryCount;
    size_t maxEntries;
} htmlEntries;

static void Grow(htmlEntries * entries) {
    entries->maxEntries *= 2;
    entries->entries = realloc(entries->entries, entries->maxEntries * sizeof(htmlEntry));
}

void HtmlEntries_Init(htmlEntries * entries) {
    entries->entryCount = 0;
    entries->maxEntries = 20;
    entries = malloc(sizeof(htmlEntry) * entries->maxEntries);
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
    if (entries->entryCount != entries->maxEntries) {
        printf("Error, entry max exceeded");
        exit(EXIT_FAILURE);
    }
    HtmlEntry_Init(&entries->entries[entries->entryCount], path, fileName);
    entries->entryCount++;
}

htmlEntry *HtmlEntries_Iterate(htmlEntries *entries, size_t *i) {

    if (*i < entries->entryCount && *i < entries->maxEntries) {
        (*i)++;
        return &entries->entries[*i];
    }
    return NULL;
}
