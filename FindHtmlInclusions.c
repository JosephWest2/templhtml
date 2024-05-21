#include "FindHtmlInclusions.h"
#include "main.h"
#include <assert.h>
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>

char *TrimWhitespace(char *s) {

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

bool GetInclusions(htmlEntries *fileEntries) {

    regex_t htmlInclusionRegex;
    int res = regcomp(&htmlInclusionRegex, "{{[ a-z._A-Z0-9]*}}", 0);
    assert(res == 0);

    
    size_t i = 0;
    htmlEntry * currentEntry;
    while ((currentEntry = HtmlEntries_Iterate(fileEntries, &i)) != NULL) {

        FILE *file = fopen(currentEntry->path, "r");
        assert(file != NULL);

        char line[1024];
        size_t lineIndex = 0;

        while ((fgets(line, sizeof(line), file)) != NULL) {

            regmatch_t match[1];
            size_t lastMatchEndIndex = 0;
            while ((regexec(&htmlInclusionRegex, line + lastMatchEndIndex, ARRAY_LENGTH(match), match, 0) == 0)) {

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

    return true;
}
