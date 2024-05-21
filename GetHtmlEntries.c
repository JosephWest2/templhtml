#include <assert.h>
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GetHtmlEntries.h"

bool IsRootDir(char *dir) { return strcmp(dir, ".") == 0 ? true : false; }

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

bool GetHtmlEntries(htmlEntries *entries, char *dirName) {

    bool success = true;

    DIR *dir = opendir(dirName);
    if (dir == NULL) {
        printf("Directory %s not found\n", dirName);
        exit(EXIT_FAILURE);
    }

    regex_t htmlRegex;
    int res = regcomp(&htmlRegex, ".html$", 0);
    assert(res == 0);

    struct dirent *dirent;
    while ((dirent = readdir(dir)) != NULL) {

        char *entryName = dirent->d_name;

        bool isHTMLFile = regexec(&htmlRegex, entryName, 0, NULL, 0) == 0 ? true : false;

        if (isHTMLFile) {
            char fileName[128];
            char path[128];
            if (IsRootDir(dirName)) {
                strcpy(path, dirent->d_name);
            } else {
                strcpy(path, dirName);
                strcat(path, "/");
                strcat(path, dirent->d_name);
            }
            strcpy(fileName, dirent->d_name);
            HtmlEntries_AddEntry(entries, path, fileName);
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
            success = GetHtmlEntries(entries, subdirName) && success;
            continue;
        }
    }
    return success;
}

