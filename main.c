#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIRECTORY_LENGTH 256
#define MAX_FILE_ENTRIES 2048
#define FILE_NAME_LENGTH 128

typedef struct {
    char path[DIRECTORY_LENGTH];
    char fileName[FILE_NAME_LENGTH];
} FileEntry;

int GetHTMLFiles(FileEntry *fileEntries, DIR *dir, char *dirName, int *i, regex_t *htmlRegex) {

    int success = 0;
    int regexResult;
    struct dirent *dirent;

    while ((dirent = readdir(dir)) != NULL) {
        regexResult = regexec(htmlRegex, dirent->d_name, 0, NULL, 0);
        if (regexResult == 0) {
            printf("dirent: %s\n", dirent->d_name);
            if (*i >= MAX_FILE_ENTRIES) {
                printf("exceeded max dirents\n");
                return -1;
            }
            strlcpy(fileEntries[*i].fileName, dirent->d_name, FILE_NAME_LENGTH);
            strlcpy(fileEntries[*i].path, dirName, DIRECTORY_LENGTH);
            strlcat(fileEntries[*i].path, "/", DIRECTORY_LENGTH);
            strlcat(fileEntries[*i].path, dirent->d_name, DIRECTORY_LENGTH);
            (*i)++;

        } else if (dirent->d_type == DT_DIR && strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0 && strlen(dirent->d_name) > 0 && dirent->d_name[0] != '.') {
            printf("directory: %s\n", dirent->d_name);

            char *subdirectory = malloc(strlen(dirent->d_name) + strlen(dirName) + 1);
            strcpy(subdirectory, dirName);
            strcat(subdirectory, "/");
            strcat(subdirectory, dirent->d_name);

            DIR *subdir = opendir(subdirectory);
            if (subdir == NULL) {
                printf("Directory %s not found\n", subdirectory);
                return -1;
            }

            success = GetHTMLFiles(fileEntries, subdir, subdirectory, i, htmlRegex);
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
    int htmlFileCount = 0;

    res = GetHTMLFiles(htmlFileEntries, dir, directory, &htmlFileCount, &htmlRegex);
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

    for (int i = 0; i < htmlFileCount; i++) {
        printf("fileEntry: fileName: %s path: %s\n", htmlFileEntries[i].fileName, htmlFileEntries[i].path);
        const FileEntry *const entry = &htmlFileEntries[i];

        regmatch_t matches[256];

        FILE *file = fopen(entry->path, "r");
        if (file == NULL) {
            printf("Unable to open file %s\n", entry->path);
            return -1;
        }

        char line[1024];

        while ((fgets(line, 60, file)) != NULL) {
            regmatch_t _matches[128];
            res = regexec(&htmlInsertRegex, line, 128, _matches, 0);
            if (res == 0) {
                printf("%s\n", line);
                printf("match\n");
            } else {
                printf("%s\n", line);
                printf("no match\n");
            }
        }
    }
}
