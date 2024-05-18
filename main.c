#include <assert.h>
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DIRECTORY_LENGTH 256
#define MAX_FILE_ENTRIES 2048
#define FILE_NAME_LENGTH 128
#define MIN(v1, v2) v1 < v2 ? v1 : v2
#define ARRAY_LENGTH(array) sizeof(array) / sizeof(array[0])

typedef struct {
    char fileName[64];
    size_t iStart;
    size_t iEnd;
} htmlInclusion;

typedef struct {
    char path[256];
    char fileName[64];
    htmlInclusion inclusions[32];
    size_t inclusionCount;
    bool visited;
} htmlFile;

typedef struct {
    htmlFile entries[128];
    size_t entryCount;
    size_t maxEntries;
} htmlFiles;

typedef struct {
    size_t array[128];
    size_t count;
} Array;

void InitFileEntry(htmlFile *entry) {
    entry->visited = false;
    entry->inclusionCount = 0;
}

htmlFiles InitFileEntries() {
    htmlFiles out;
    out.entryCount = 0;
    out.maxEntries = ARRAY_LENGTH(out.entries);
    return out;
}

void Array_Add(Array *arr, size_t n) {
    arr->array[arr->count] = n;
    arr->count++;
}

bool Array_Contains(Array *arr, size_t n) {
    for (size_t i = 0; i < arr->count; i++) {
        if (arr->array[i] == n) {
            return true;
        }
    }
    return false;
}

bool IsValidDirectory(struct dirent *dirent) {

    if (dirent->d_type != DT_DIR)
        return false;
    if (strcmp(dirent->d_name, ".") == 0)
        return false;
    if (strcmp(dirent->d_name, "..") == 0)
        return false;
    if (strlen(dirent->d_name) == 0)
        return false;

    return true;
}

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

bool IsRootDir(char *dir) { return strcmp(dir, ".") == 0 ? true : false; }

bool GetHTMLFiles(htmlFiles *fileEntries, DIR *dir, char *dirName, regex_t *htmlFileRegex) {

    bool success = true;

    struct dirent *dirent;
    while ((dirent = readdir(dir)) != NULL) {

        bool isHTMLFile = regexec(htmlFileRegex, dirent->d_name, 0, NULL, 0) == 0 ? true : false;

        if (isHTMLFile) {
            htmlFile *current = &fileEntries->entries[fileEntries->entryCount];
            InitFileEntry(current);
            if (IsRootDir(dirName)) {
                strcpy(current->path, dirent->d_name);
            } else {
                strcpy(current->path, dirName);
                strcat(current->path, "/");
                strcat(current->path, dirent->d_name);
            }
            strcpy(current->fileName, dirent->d_name);
            fileEntries->entryCount++;
            current->inclusionCount++;
            continue;
        }

        if (IsValidDirectory(dirent)) {
            char subdirName[128];
            if (IsRootDir(dirName)) {
                strcpy(subdirName, dirent->d_name);
            } else {
                strcpy(subdirName, dirName);
                strcat(subdirName, "/");
                strcat(subdirName, dirent->d_name);
            }
            DIR *subdir = opendir(subdirName);
            assert(subdir != NULL);
            success = GetHTMLFiles(fileEntries, subdir, subdirName, htmlFileRegex) && success;
            continue;
        }
    }
    return success;
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

htmlFile *GetInclusion(htmlFiles *files, htmlInclusion *inclusion) {

    for (size_t i = 0; i < files->entryCount; i++) {
        htmlFile *current = &files->entries[i];
        printf("current: %s inclusion: %s\n", current->fileName, inclusion->fileName);
        assert(current != NULL);
        if (strcmp(current->fileName, inclusion->fileName) == 0) {
            return current;
        }
    }

    return NULL;
}

bool GetInclusions(htmlFiles *fileEntries) {

    regex_t htmlInsertRegex;
    int res = regcomp(&htmlInsertRegex, "{{[ a-z._A-Z0-9]*}}", 0);
    assert(res == 0);

    for (size_t i = 0; i < fileEntries->entryCount; i++) {
        htmlFile *currentFileEntry = &fileEntries->entries[i];

        FILE *file = fopen(currentFileEntry->path, "r");
        assert(file != NULL);

        char line[1024];
        size_t lineIndex = 0;

        while ((fgets(line, sizeof(line), file)) != NULL) {

            regmatch_t match[1];
            size_t lastMatchEndIndex = 0;
            while ((regexec(&htmlInsertRegex, line + lastMatchEndIndex, ARRAY_LENGTH(match), match, 0) == 0)) {

                htmlInclusion *currentMatch = &currentFileEntry->inclusions[currentFileEntry->inclusionCount];
                currentMatch->iStart = match[0].rm_so + lastMatchEndIndex + lineIndex;
                currentMatch->iEnd = match[0].rm_eo + lastMatchEndIndex + lineIndex;
                printf("match at %zu -> %zu: %s\n", currentMatch->iStart, currentMatch->iEnd, line);
                size_t innerValueLength = MIN(match[0].rm_eo - match[0].rm_so - 4, sizeof(currentMatch->fileName));
                printf("innerValueLength %zu\n", innerValueLength);
                memcpy(currentMatch->fileName, line + lastMatchEndIndex + match[0].rm_so + 2, innerValueLength);
                currentMatch->fileName[innerValueLength] = '\0';
                printf("CURRENTMATCH: %s\n", currentMatch->fileName);
                lastMatchEndIndex += match[0].rm_eo;
                currentFileEntry->inclusionCount++;

                if ((strcmp(currentMatch->fileName, currentFileEntry->fileName)) == 0) {
                    printf("Recursive inclusion found at %s\n", currentFileEntry->fileName);
                    return false;
                }
            }
            printf("no more matches found on line %s\n", line);
            lineIndex += strlen(line);
        }
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

    DIR *dir = opendir(directory);
    if (dir == NULL) {
        printf("Directory %s not found\n", directory);
        return -1;
    }

    regex_t htmlRegex;
    int res = regcomp(&htmlRegex, ".html$", 0);
    assert(res == 0);

    htmlFiles fileEntries = InitFileEntries();
    res = GetHTMLFiles(&fileEntries, dir, directory, &htmlRegex);
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
