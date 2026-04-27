#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    // Variables used
    DIR *directory;
    struct dirent *entry;
    const char *dir_path = ".";
    int opt;
    int a_flag = 0;

    // Check if the -a flag is set
    while ((opt = getopt(argc, argv, "a")) != -1) {
        switch(opt) {
            case 'a':
                a_flag = 1;
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    if (optind < argc) {
        dir_path = argv[optind];
    }

    // Set errno to 0
    errno = 0;

    // Open the directory stream
    directory = opendir(dir_path);

    if (directory == NULL) {
        perror(argv[0]);
        return 1;
    }

    // Read all entries in the directory stream
    while ((entry = readdir(directory)) != NULL) {
        if (strlen(entry->d_name) > 0 && entry->d_name[0] != '.')
            printf("%s  ", entry->d_name);
    }
    printf("\n");

    // Close the directory stream
    closedir(directory);

    if (errno != 0) {
        perror(argv[0]);
        return 1;
    }

    return 0;
}