#include "BuildOutput.h"
#include "main.h"
#include <assert.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

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

bool BuildOutput(htmlEntries *fileEntries) {

    bool success = true;

    regex_t htmlInclusionRegex;
    int res = regcomp(&htmlInclusionRegex, "{{[ a-z._A-Z0-9]*}}", 0);
    assert(res == 0);

    size_t i = 0;
    htmlEntry *currentEntry;
    while ((currentEntry = HtmlEntries_Iterate(fileEntries, &i)) != NULL) {

        FILE *srcFile = fopen(currentEntry->path, "r");
        assert(srcFile != NULL);

        char destPath[128];
        _GetBuildPath(currentEntry->path, destPath);
        _BuildDirectory(destPath);
        FILE *destFile = fopen(destPath, "w");
        assert(destFile != NULL);

        char line[1024];
        size_t lineIndex = 0;
        while ((fgets(line, sizeof(line), srcFile)) != NULL) {

            char * p = line;
            regmatch_t match[1];
            while ((regexec(&htmlInclusionRegex, p, ARRAY_LENGTH(match), match, 0) == 0)) {

                htmlInclusion *inclusion = &currentFileEntry->inclusions[currentFileEntry->inclusionCount];
                size_t iStart = match[0].rm_so + lastMatchEndIndex + lineIndex;
                size_t iEnd = match[0].rm_eo + lastMatchEndIndex + lineIndex;
                size_t innerValueLength = match[0].rm_eo - match[0].rm_so - 4;
                line[innerValueLength] = '\0';
                char *fileNameUntrimmed = line + lastMatchEndIndex + match[0].rm_so + 2;
                char *fileName = TrimWhitespace(fileNameUntrimmed);
                lastMatchEndIndex += match[0].rm_eo;
                currentFileEntry->inclusionCount++;

                if ((strcmp(inclusion->fileName, currentFileEntry->fileName)) == 0) {
                    printf("Recursive inclusion found at %s\n", currentFileEntry->fileName);
                    return false;
                }
            }
            lineIndex += strlen(line);
        }
    }

    return success;
}
