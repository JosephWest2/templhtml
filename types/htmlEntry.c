#include "htmlEntry.h"
#include <stdlib.h>
#include <string.h>

static void _GrowInclusions(htmlEntry *entry) {
    entry->inclusionMax *= 2;
    entry->inclusions = realloc(entry->inclusions, sizeof(htmlInclusion) * entry->inclusionMax);
}

void HtmlEntry_Init(htmlEntry *entry, char *path, char *fileName) {
    entry->built = false;
    entry->inclusionCount = 0;
    entry->inclusionMax = 10;
    entry->inclusions = malloc(sizeof(htmlInclusion) * entry->inclusionMax);
    strcpy(entry->path, path);
    strcpy(entry->fileName, fileName);
}

void HtmlEntry_AddHtmlInclusion(htmlEntry *entryToAddTo, size_t iStart, size_t iEnd, htmlEntry *referencedEntry) {
    if (entryToAddTo->inclusionCount + 1 >= entryToAddTo->inclusionMax) {
        _GrowInclusions(entryToAddTo);
    }
    htmlInclusion *inc = &entryToAddTo->inclusions[entryToAddTo->inclusionCount];
    entryToAddTo->inclusionCount++;
    inc->entry = referencedEntry;
    inc->iEnd = iEnd;
    inc->iStart = iStart;
}

htmlInclusion *HtmlEntry_IterateInclusions(htmlEntry *entry, size_t *i) {
    if (*i < entry->inclusionCount) {
        (*i)++;
        return &entry->inclusions[*i];
    }
    return NULL;
}
