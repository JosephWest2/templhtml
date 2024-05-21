#include "main.h"
#include "GetHtmlEntries.h"
#include <assert.h>
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int GetBuildableInclusionIndex(htmlFiles *files, htmlInclusion *inclusion) {

    for (size_t i = 0; i < files->entryCount; i++) {
        htmlFile *current = &files->entries[i];
        assert(current != NULL);
        if (strcmp(current->fileName, inclusion->fileName) == 0) {
            if (current->visited)
                return -1;
            return i;
        }
    }

    return -1;
}

bool CreateFilePath(char *path) {

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
    return true;
}

bool PasteFile(htmlFile *fileEntry, FILE *dest) {

    char srcPath[128] = "build/";
    strcat(srcPath, fileEntry->path);
    FILE *src = fopen(srcPath, "r");
    assert(src != NULL);

    char c;
    while ((c = fgetc(src)) != EOF) {
        fputc(c, dest);
    }
    return true;
}

int BuildOutputFile(htmlFiles *htmlFiles, size_t htmlFileIndex, Array *parents) {

    if (Array_Contains(parents, htmlFileIndex)) {
        printf("LOOP DETECTED\n");
        exit(EXIT_FAILURE);
        return -1;
    }

    // Init pointers and build necessary directories
    htmlFile *fileEntry = &htmlFiles->entries[htmlFileIndex];
    fileEntry->visited = true;
    FILE *sourceFile = fopen(fileEntry->path, "r");
    char destinationPath[128] = "build/";
    strcat(destinationPath, fileEntry->path);
    CreateFilePath(destinationPath);
    FILE *outputFile = fopen(destinationPath, "w");
    if (outputFile == NULL || sourceFile == NULL) {
        printf("error at BuildOutputFile\n");
        return -1;
    }

    // Build Dependencies if not built
    for (int i = 0; i < fileEntry->inclusionCount; i++) {
        int buildableInclusionIndex = GetBuildableInclusionIndex(htmlFiles, &fileEntry->inclusions[i]);
        if (buildableInclusionIndex != -1) {
            Array_Add(parents, htmlFileIndex);
            BuildOutputFile(htmlFiles, buildableInclusionIndex, parents);
        }
    }

    // Build the build file
    size_t currentInclusionIndex = 0;
    printf("currentInclusionIndex: %ld, fileInclusionCount: %ld\n", currentInclusionIndex, fileEntry->inclusionCount);
    htmlInclusion *currentInclusion = &fileEntry->inclusions[currentInclusionIndex];
    printf("currentFile: %s, currentInclusion: %s\n", fileEntry->fileName, currentInclusion->fileName);

    while (true) {
        long l = ftell(sourceFile);

        if (currentInclusionIndex < fileEntry->inclusionCount && l == currentInclusion->iStart) {

            printf("FOUND\n");

            // Find associated File Entry
            htmlFile *inclusion = GetInclusion(htmlFiles, currentInclusion);

            if (inclusion != NULL) {
                PasteFile(inclusion, outputFile);
                fseek(sourceFile, currentInclusion->iEnd - currentInclusion->iStart, l);
            }
            currentInclusionIndex++;
            if (currentInclusionIndex < fileEntry->inclusionCount)
                currentInclusion = &fileEntry->inclusions[currentInclusionIndex];
            continue;
        }

        char c = fgetc(sourceFile);
        if (c == EOF) {
            return 0;
        }
        fputc(c, outputFile);
    }
}

int main(int argc, char **argv) {

    char directory[DIRECTORY_LENGTH] = ".";

    if (argc != 0) {
        if (strlen(argv[0]) < DIRECTORY_LENGTH) {
            strcpy(argv[0], directory);
        } else {
            memcpy(directory, argv[0], DIRECTORY_LENGTH - 2);
            directory[DIRECTORY_LENGTH - 1] = '\0';
        }
    }

    htmlEntries htmlEntries;
    HtmlEntries_Init(&htmlEntries);

    bool res = GetHtmlEntries(&htmlEntries, directory);
    assert(res == true);

    bool result = GetInclusions(&fileEntries);
    if (result == false)
        return -1;

    for (size_t i = 0; i < fileEntries.entryCount; i++) {
        if (fileEntries.entries[i].visited)
            continue;
        Array parents;
        parents.count = 0;
        BuildOutputFile(&fileEntries, i, &parents);
    }

    return 0;
}
