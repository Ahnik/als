#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

// Maximum length for a complete pathname
#define PATH_MAX 4096

// Function to print the long version of a directory entry
void print_long_list(const char *pathname);

// Function to join the directory name and the filename together to form pathname
char *get_pathname(const char *dir_path, const char *filename);

int main(int argc, char **argv) {
    // Variables used
    DIR *directory;
    struct dirent *entry;
    const char *dir_path = ".";
    int opt;

    // Flags used
    bool a_flag = false;
    bool l_flag = false;

    // Parse the input flags
    while ((opt = getopt(argc, argv, "al")) != -1) {
        switch (opt) {
            case 'a':
                a_flag = true;
                break;
            case 'l':
                l_flag = true;
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    // Get the directory name from the argument list if it is present (The first directory name it can find)
    if (optind < argc) {
        dir_path = argv[optind];
    }

    // Set errno to 0
    errno = 0;

    // Open the directory stream
    directory = opendir(dir_path);

    if (directory == NULL) {
        fprintf(stderr, "%s: cannot access '%s': %s\n", argv[0], dir_path, strerror(errno));
        return 1;
    }

    // Read all entries in the directory stream
    while ((entry = readdir(directory)) != NULL) {
        if (a_flag || (entry->d_name[0] != '.')) {
            if (l_flag)
                print_long_list(get_pathname(dir_path, entry->d_name));
            else if (strlen(entry->d_name) > 0)
                printf("%s  ", entry->d_name);
        }
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

void print_long_list(const char *pathname) {
    // Struct for storing file stats
    struct stat file_stat;

    // Obtain the file stats
    if (lstat(pathname, &file_stat) == -1) {
        perror("Error while obtaining file stats");
        exit(1);
    }

    /* TODO: Write logic to print the long version of ls of a file entry */
    printf("%s\n", pathname);
}

char *get_pathname(const char *dir_path, const char *filename) {
    // Make the buffer static so that the memory persists for the lifetime of the program
    static char pathname[PATH_MAX];
    snprintf(pathname, PATH_MAX, "%s", dir_path);

    // Add the '/' if there is none
    if (dir_path[strlen(dir_path) - 1] != '/')
        strncat(pathname, "/", PATH_MAX - strlen(pathname) - 1);

    // Concatenate the filename
    strncat(pathname, filename, PATH_MAX - strlen(pathname) - 1);

    return pathname;
}