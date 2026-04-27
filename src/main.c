#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    DIR *directory;
    struct dirent *entry;

    // Set errno to 0
    errno = 0;

    // Open the directory stream
    directory = opendir(".");

    if (directory == NULL) {
        perror("Error opening directory directory");
        return EXIT_FAILURE;
    }

    // Read all entries in the directory stream
    while ((entry = readdir(directory)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    if (errno != 0) {
        perror("Error reading directory");
        return EXIT_FAILURE;
    }

    closedir(directory);
    return EXIT_SUCCESS;
}