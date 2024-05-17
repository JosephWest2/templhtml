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

typedef struct FileInclusion {
    char innerValue[64];
    size_t iStart;
    size_t iEnd;
} FileInclusion;

typedef struct FileEntry {
    char path[256];
    char fileName[64];
    FileInclusion inclusions[32];
    size_t inclusionCount;
    bool built;
} FileEntry;

int GetHTMLFiles(FileEntry *fileEntries, size_t maxFileEntries, DIR *dir, char *dirName, size_t *fileCount, regex_t *htmlFileRegex) {

    int success = 0;
    int regexResult;
    struct dirent *directoryEntry;
    printf("YO\n");

    while ((directoryEntry = readdir(dir)) != NULL) {
        regexResult = regexec(htmlFileRegex, directoryEntry->d_name, 0, NULL, 0);
        if (regexResult == 0) {
            printf("dirent: %s\n", directoryEntry->d_name);
            if (*fileCount >= maxFileEntries) {
                printf("exceeded max dirents\n");
                return -1;
            }
            strlcpy(fileEntries[*fileCount].fileName, directoryEntry->d_name, FILE_NAME_LENGTH);
            if (strcmp(dirName, ".")) {
                strlcpy(fileEntries[*fileCount].path, dirName, DIRECTORY_LENGTH);
                strlcat(fileEntries[*fileCount].path, "/", DIRECTORY_LENGTH);
                strlcat(fileEntries[*fileCount].path, directoryEntry->d_name, DIRECTORY_LENGTH);
            } else {
                strlcpy(fileEntries[*fileCount].path, directoryEntry->d_name, DIRECTORY_LENGTH);
            }
            fileEntries[*fileCount].built = false;
            fileEntries[*fileCount].inclusionCount++;
            (*fileCount)++;

        } else if (directoryEntry->d_type == DT_DIR && strcmp(directoryEntry->d_name, ".") != 0 && strcmp(directoryEntry->d_name, "..") != 0 && strlen(directoryEntry->d_name) > 0 &&
                   directoryEntry->d_name[0] != '.') {
            printf("directory: %s\n", directoryEntry->d_name);

            char *subdirectoryName = malloc(strlen(directoryEntry->d_name) + strlen(dirName) + 1);
            if (strcmp(dirName, ".")) {
                strcpy(subdirectoryName, dirName);
                strcat(subdirectoryName, "/");
                strcat(subdirectoryName, directoryEntry->d_name);
            } else {
                strcpy(subdirectoryName, directoryEntry->d_name);
            }

            DIR *subDIR = opendir(subdirectoryName);
            if (subDIR == NULL) {
                printf("Directory %s not found\n", subdirectoryName);
                return -1;
            }

            success = GetHTMLFiles(fileEntries, maxFileEntries, subDIR, subdirectoryName, fileCount, htmlFileRegex);
        }
    }
    return success;
}

int CreateFilePath(char *path) {

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
    return 0;
}

int PasteFile(FileEntry *fileEntry, FILE *dest) {

    printf("PASTING\n");
    char srcPath[128] = "build/";
    strcat(srcPath, fileEntry->path);
    FILE *src = fopen(srcPath, "r");
    assert(src != NULL);

    char c;
    while ((c = fgetc(src)) != EOF) {
        fputc(c, dest);
    }
    return 0;
}

int BuildOutputFile(FileEntry *htmlFileEntries, size_t htmlFileCount, size_t htmlFileIndex) {

    // Init pointers and build necessary directories
    FileEntry *fileEntry = &htmlFileEntries[htmlFileIndex];
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
        for (int j = 0; j < htmlFileCount; j++) {
            if (fileEntry->inclusions[i].innerValue == htmlFileEntries[j].fileName && !htmlFileEntries[j].built) {
                BuildOutputFile(htmlFileEntries, htmlFileCount, j);
            }
        }
    }

    // Build the build file
    size_t currentInclusionIndex = 0;
    printf("currentInclusionIndex: %ld, fileInclusionCount: %ld\n", currentInclusionIndex, fileEntry->inclusionCount);
    FileInclusion *currentInclusion = &fileEntry->inclusions[currentInclusionIndex];

    while (true) {

        long l = ftell(sourceFile);
        printf("%ld : %ld\n", l, currentInclusion->iStart);

        if (currentInclusionIndex < fileEntry->inclusionCount && l == currentInclusion->iStart) {

            printf("FOUND\n");

            // Find associated File Entry
            FileEntry *inclusionEntry = NULL;
            for (int i = 0; i < htmlFileCount; i++) {
                printf("inclusionInnerValue %s, fileName %s\n", currentInclusion->innerValue, htmlFileEntries[i].fileName);
                if (htmlFileEntries[i].fileName == currentInclusion->innerValue) {
                    inclusionEntry = &htmlFileEntries[i];
                }
            }
            assert(inclusionEntry != NULL);

            // Paste in the file
            PasteFile(inclusionEntry, outputFile);

            // move pointer to after the inclusion
            fseek(sourceFile, currentInclusion->iEnd - currentInclusion->iStart, l);
            currentInclusionIndex++;
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

    regex_t htmlRegex;
    int res = regcomp(&htmlRegex, ".html$", 0);
    if (res != 0) {
        printf("htmlRegex compilation failed\n");
        return -1;
    }

    DIR *dir = opendir(directory);
    if (dir == NULL) {
        printf("Directory %s not found\n", directory);
        return -1;
    }

    FileEntry htmlFileEntries[MAX_FILE_ENTRIES];
    size_t htmlFileCount = 0;

    res = GetHTMLFiles(htmlFileEntries, ARRAY_LENGTH(htmlFileEntries), dir, directory, &htmlFileCount, &htmlRegex);
    if (res == -1) {
        printf("Error at GetHTMLFiles\n");
        return -1;
    }

    regex_t htmlInsertRegex;
    res = regcomp(&htmlInsertRegex, "{{[ a-z._A-Z0-9]*}}", 0);
    if (res != 0) {
        printf("htmlInsertRegex compilation failed\n");
        return -1;
    }

    res = regexec(&htmlInsertRegex, "al{{ bbb0b }}skdf{{jk999asdfjkha}}afdslka", 0, NULL, 0);
    if (res == 0) {
        printf("test pass for htmlinsertregex\n");
    } else {
        printf("test failed for htmlInsertRegex\n");
    }

    for (size_t i = 0; i < htmlFileCount; i++) {
        printf("fileEntry: fileName: %s path: %s\n", htmlFileEntries[i].fileName, htmlFileEntries[i].path);
        FileEntry *const currentFileEntry = &htmlFileEntries[i];
        currentFileEntry->inclusionCount = 0;
        currentFileEntry->built = false;

        FILE *file = fopen(currentFileEntry->path, "r");
        if (file == NULL) {
            printf("Unable to open file %s\n", currentFileEntry->path);
            return -1;
        }

        char line[1024];
        size_t lineIndex = 0;

        while ((fgets(line, sizeof(line), file)) != NULL) {

            regmatch_t match[1];
            size_t lastMatchEndIndex = 0;
            while ((regexec(&htmlInsertRegex, line + lastMatchEndIndex, ARRAY_LENGTH(match), match, 0) == 0)) {

                FileInclusion *currentMatch = &currentFileEntry->inclusions[currentFileEntry->inclusionCount];
                currentMatch->iStart = match[0].rm_so + lastMatchEndIndex + lineIndex;
                currentMatch->iEnd = match[0].rm_eo + lastMatchEndIndex + lineIndex;
                printf("match at %zu -> %zu: %s\n", currentMatch->iStart, currentMatch->iEnd, line);
                size_t innerValueLength = MIN(match[0].rm_eo - match[0].rm_so - 4, sizeof(currentMatch->innerValue));
                printf("innerValueLength %zu\n", innerValueLength);
                memcpy(currentMatch->innerValue, line + lastMatchEndIndex + match[0].rm_so + 2, innerValueLength);
                currentMatch->innerValue[innerValueLength] = '\0';
                printf("CURRENTMATCH: %s\n", currentMatch->innerValue);

                if ((strcmp(currentMatch->innerValue, currentFileEntry->fileName)) == 0) {
                    printf("Recursive inclusion found at %s\n", currentFileEntry->fileName);
                    return -1;
                }

                lastMatchEndIndex += match[0].rm_eo;
                currentFileEntry->inclusionCount++;
            }
            printf("no more matches found on line %s\n", line);
            lineIndex += strlen(line);
        }
    }

    size_t visitedIndexes[256];
    size_t visitedIndexesLength = 0;
    size_t htmlFileIndex = 0;

    BuildOutputFile(htmlFileEntries, htmlFileCount, htmlFileIndex);

    return 0;
}
