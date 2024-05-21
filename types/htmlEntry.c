#include "htmlEntry.h"
#include <string.h>

void HtmlEntry_Init(htmlEntry *entry, char *path, char *fileName) {

    static size_t id = 0;

    entry->id = id;
    id++;
    entry->built = false;
    strcpy(entry->path, path);
    strcpy(entry->fileName, fileName);
}
