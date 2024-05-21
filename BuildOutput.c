#include <assert.h>
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "BuildOutput.h"
#include "main.h"

static void _GetBuildPath(const char *const srcPath, char *out) {
    strcpy(out, "build/");
    strcat(out, srcPath);
}

static void _BuildDirectory(const char *const path) {
    char buffer[256];
    size_t bufferIndex = 0;
    size_t len = strlen(path);
    for (size_t i = 0; i < len; i++) {
        char c = path[i];
        if (c == '/' || c == '\\') {
            buffer[bufferIndex] = '\0';
            mkdir(buffer, 0700);
            bufferIndex = 0;
            continue;
        }
        buffer[bufferIndex] = path[i];
        bufferIndex++;
    }
}
static char *_TrimWhitespace(char *s) {

    size_t len = strlen(s);
    char *start = s;
    char *end = s + len - 1;

    while (isspace((unsigned char)*start)) {
        start++;
        if (start >= end) {
            *start = '\0';
            return start;
        }
    }
    while (isspace((unsigned char)*end)) {
        end--;
        if (end <= start) {
            *start = '\0';
            return start;
        }
    }
    *end = '\0';
    return start;
}

static void _BuildFile(htmlEntries *entries, htmlEntry *entry, ArrayList *parents, const regex_t *const htmlInclusionRegex) {

    FILE *srcFile = fopen(entry->path, "r");
    assert(srcFile != NULL);

    char destPath[128];
    _GetBuildPath(entry->path, destPath);
    _BuildDirectory(destPath);
    FILE *destFile = fopen(destPath, "w");
    assert(destFile != NULL);

    char line[1024];
    size_t lineIndex = 0;
    while ((fgets(line, sizeof(line), srcFile)) != NULL) {

        char *p = line;
        regmatch_t match[1];
        while ((regexec(htmlInclusionRegex, p, ARRAY_LENGTH(match), match, 0) == 0)) {

            ArrayList *newParents;
            ArrayList_CreateAndClone(parents, newParents);
            ArrayList_Append(newParents, entry->id);

            line[match[0].rm_so] = '\0';
            line[match[0].rm_eo - 2] = '\0';
            fputs(p, destFile);
            p += match[0].rm_eo;

            char fileName[64];
            strcpy(fileName, line + match[0].rm_so + 2);
            char *fileNameTrimmed = _TrimWhitespace(fileName);
            printf("fileName %s\n", fileName);

            htmlEntry *inclusion = GetAssociatedHtmlEntry(entries, fileNameTrimmed);
            if (inclusion == NULL) {
                printf("file %s not found\n", fileNameTrimmed);
                ArrayList_Destroy(newParents);
                continue;
            }
            if (ArrayList_Contains(newParents, inclusion->id)) {
                printf("inclusion loop found at %s\n", inclusion->path);
                exit(EXIT_FAILURE);
            }
            if (!inclusion->built) {
                _BuildFile(entries, inclusion, newParents, htmlInclusionRegex);
            }
            char builtIncluisonPath[128];
            _GetBuildPath(inclusion->path, builtIncluisonPath);
            FILE *inclusionFile = fopen(builtIncluisonPath, "r");
            assert(inclusionFile != NULL);

            char inclusionLine[1024];
            while ((fgets(inclusionLine, sizeof(inclusionLine), inclusionFile)) != NULL) {
                fputs(inclusionLine, destFile);
            }
            ArrayList_Destroy(newParents);
        }
        fputs(p, destFile);
    }

    entry->built = true;
}

bool BuildOutput(htmlEntries *fileEntries) {

    bool success = true;

    regex_t htmlInclusionRegex;
    int res = regcomp(&htmlInclusionRegex, "{{[ a-z._A-Z0-9]*}}", 0);
    assert(res == 0);

    size_t i = 0;
    htmlEntry *currentEntry;
    while ((currentEntry = HtmlEntries_Iterate(fileEntries, &i)) != NULL) {

        ArrayList parents;
        ArrayList_Create(&parents, 10);
        _BuildFile(fileEntries, currentEntry, &parents, &htmlInclusionRegex);
        ArrayList_Destroy(&parents);
    }

    return success;
}
