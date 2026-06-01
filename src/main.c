#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"

int main(int argc, char **argv) {
    // Variables used
    DIR *directory;
    struct dirent *entry;
    const char *dir_path = ".";
    int opt;
    // size_t links = 0;
    // int max_len[NUM_VARIABLE_FIELDS] = {0};
    // FileStats *file_stats = NULL;

    // Flags used
    bool a_flag = false;
    bool l_flag = false;
    bool i_flag = false;

    // Parse the input flags
    while ((opt = getopt(argc, argv, "ali")) != -1) {
        switch (opt) {
            case 'a':
                a_flag = true;
                break;
            case 'l':
                l_flag = true;
                break;
            case 'i':
                i_flag = true;
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
            if (l_flag) {
                // Get the file stats
                FileStats file_stats = get_file_stats(dir_path, entry);

                // Print the inode number if i flag is enabled
                if (i_flag)
                    printf("%ld ", file_stats.inode);

                // Print the rest of the fields
                printf("%s %ld %s %s %ld %s %s\n", 
                    file_stats.permission_string,
                    file_stats.links,
                    file_stats.username,
                    file_stats.groupname,
                    file_stats.size,
                    file_stats.last_modification,
                    file_stats.filename
                );
            }
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