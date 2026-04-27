#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    // Variables used
    DIR *directory;
    struct dirent *entry;
    const char *dir_path;

    // Set the directory path
    if (argc == 1)
        dir_path = ".";
    else
        dir_path = argv[1];

    // Set errno to 0
    errno = 0;

    // Open the directory stream
    directory = opendir(dir_path);

    if (directory == NULL) {
        perror("Error opening directory directory");
        return EXIT_FAILURE;
    }

    // Read all entries in the directory stream
    while ((entry = readdir(directory)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    // Close the directory stream
    closedir(directory);

    if (errno != 0) {
        perror("Error reading directory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}