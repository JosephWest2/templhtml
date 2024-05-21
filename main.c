#include "main.h"
#include "BuildOutput.h"
#include "GetHtmlEntries.h"
#include <assert.h>
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


int main(int argc, char **argv) {

    char directory[DIRECTORY_LENGTH] = ".";

    if (argc >= 2) {
        if (strlen(argv[1]) < DIRECTORY_LENGTH) {
            strcpy(directory, argv[1]);
        } else {
            memcpy(directory, argv[1], DIRECTORY_LENGTH - 2);
            directory[DIRECTORY_LENGTH - 1] = '\0';
        }
    }

    htmlEntries htmlEntries;
    HtmlEntries_Init(&htmlEntries);

    bool res = GetHtmlEntries(&htmlEntries, directory);
    assert(res == true);

    BuildOutput(&htmlEntries);
    printf("build complete at '%s'\n", directory);

    return 0;
}
