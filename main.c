#include "main.h"
#include "BuildOutput.h"
#include "GetHtmlEntries.h"
#include <assert.h>
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>


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

    BuildOutput(&htmlEntries);

    return 0;
}
