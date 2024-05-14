#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIRECTORY_LENGTH 256
#define MAX_FILE_ENTRIES 2048
#define FILE_NAME_LENGTH 128
#define MIN(v1, v2) v1 < v2 ? v1 : v2
#define ARRAY_LENGTH(array) sizeof(array) / sizeof(array[0])

typedef struct {
    char path[DIRECTORY_LENGTH];
    char fileName[FILE_NAME_LENGTH];
} FileEntry;

typedef struct {

    FileEntry *parentFile;
    char innerValue[256];
    size_t iStart;
    size_t iEnd;

} Match;

int GetHTMLFiles(FileEntry *fileEntries, size_t maxFileEntries, DIR *dir, char *dirName, size_t *fileCount, regex_t *htmlFileRegex) {

    int success = 0;
    int regexResult;
    struct dirent *directoryEntry;

    while ((directoryEntry = readdir(dir)) != NULL) {
        regexResult = regexec(htmlFileRegex, directoryEntry->d_name, 0, NULL, 0);
        if (regexResult == 0) {
            printf("dirent: %s\n", directoryEntry->d_name);
            if (*fileCount >= maxFileEntries) {
                printf("exceeded max dirents\n");
                return -1;
            }
            strlcpy(fileEntries[*fileCount].fileName, directoryEntry->d_name, FILE_NAME_LENGTH);
            strlcpy(fileEntries[*fileCount].path, dirName, DIRECTORY_LENGTH);
            strlcat(fileEntries[*fileCount].path, "/", DIRECTORY_LENGTH);
            strlcat(fileEntries[*fileCount].path, directoryEntry->d_name, DIRECTORY_LENGTH);
            (*fileCount)++;

        } else if (directoryEntry->d_type == DT_DIR && strcmp(directoryEntry->d_name, ".") != 0 && strcmp(directoryEntry->d_name, "..") != 0 && strlen(directoryEntry->d_name) > 0 &&
                   directoryEntry->d_name[0] != '.') {
            printf("directory: %s\n", directoryEntry->d_name);

            char *subdirectoryName = malloc(strlen(directoryEntry->d_name) + strlen(dirName) + 1);
            strcpy(subdirectoryName, dirName);
            strcat(subdirectoryName, "/");
            strcat(subdirectoryName, directoryEntry->d_name);

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
    res = regcomp(&htmlInsertRegex, "{{[a-z._A-Z0-9]*}}", 0);
    if (res != 0) {
        printf("htmlInsertRegex compilation failed\n");
        return -1;
    }

    res = regexec(&htmlInsertRegex, "al{{bbb0b}}skdf{{jk999asdfjkha}}afdslka", 0, NULL, 0);
    if (res == 0) {
        printf("test pass for htmlinsertregex\n");
    } else {
        printf("test failed for htmlInsertRegex\n");
    }

    Match matches[1024];
    size_t matchCount = 0;

    for (size_t i = 0; i < htmlFileCount; i++) {
        printf("fileEntry: fileName: %s path: %s\n", htmlFileEntries[i].fileName, htmlFileEntries[i].path);
        FileEntry *const currentFileEntry = &htmlFileEntries[i];

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

                Match *currentMatch = &matches[matchCount];
                currentMatch->parentFile = currentFileEntry;
                printf("match at %zu -> %zu: %s\n", match[0].rm_so + lastMatchEndIndex + lineIndex, match[0].rm_eo + lastMatchEndIndex + lineIndex, line);
                currentMatch->iStart = match[0].rm_so + lastMatchEndIndex + lineIndex;
                currentMatch->iEnd = match[0].rm_eo + lastMatchEndIndex + lineIndex;
                size_t innerValueLength = MIN(match[0].rm_eo - match[0].rm_so - 4, sizeof(currentMatch->innerValue));
                printf("innerValueLength %zu\n", innerValueLength);
                memcpy(currentMatch->innerValue, line + lastMatchEndIndex + match[0].rm_so + 2, innerValueLength);
                currentMatch->innerValue[innerValueLength] = '\0';

                if ((strcmp(currentMatch->innerValue, currentFileEntry->fileName)) == 0) {
                    printf("Recursive inclusion found at %s\n", currentFileEntry->fileName);
                    return -1;
                }

                lastMatchEndIndex += match[0].rm_eo;
                matchCount++;
            }
            printf("no more matches found on line %s\n", line);
            lineIndex += strlen(line);
        }
    }

    for (size_t i = 0; i < matchCount; i++) {
        printf("%zu %zu %s\n", matches[i].iStart, matches[i].iEnd, matches[i].innerValue);



    }
}
