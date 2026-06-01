#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"

int main(int argc, char **argv) {
    // Variables used
    DIR *directory;
    struct dirent *entry;
    const char *dir_path = ".";
    int opt;
    size_t links = 0;
    size_t max_len[NUM_VARIABLE_FIELDS] = {0};
    FileStats **file_stats = NULL;

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
                links++;
                // Check whether size of the memory length would cause an overflow
                if (links > SIZE_MAX / sizeof(FileStats *)) {
                    fprintf(stderr, "%s: memory size length overflow!\n", argv[0]);
                    for (size_t i = 0; i < links-1; i++) {
                        free(file_stats[i]);
                    }
                    free(file_stats);
                    closedir(directory);
                    return 1;
                }

                // Resize the buffer for storing file metadata
                file_stats = (FileStats **) realloc(file_stats, links * sizeof(FileStats *));
                if (file_stats == NULL) {
                    perror(argv[0]);
                    closedir(directory);
                    return 1;
                }

                // Get all the file metadata
                file_stats[links-1] = get_file_stats(dir_path, entry);

                // Compare the inode number, links number, username, groupname and size of the file
                if (no_of_digits(file_stats[links-1]->inode) > max_len[0])
                    max_len[0] = no_of_digits(file_stats[links-1]->inode);
                if (no_of_digits(file_stats[links-1]->links) > max_len[1])
                    max_len[1] = no_of_digits(file_stats[links-1]->links);
                if (strlen(file_stats[links-1]->username) > max_len[2])
                    max_len[2] = strlen(file_stats[links-1]->username);
                if (strlen(file_stats[links-1]->groupname) > max_len[3])
                    max_len[3] = strlen(file_stats[links-1]->groupname);
                if (no_of_digits(file_stats[links-1]->size) > max_len[4])
                    max_len[4] = no_of_digits(file_stats[links-1]->size);
            }
            else if (strlen(entry->d_name) > 0)
                printf("%s  ", entry->d_name);
        }
    }

    // Print all the data about each file
    if (l_flag) {
        for (size_t i = 0; i < links; i++) {
            if (i_flag) {
                for (size_t j = no_of_digits(file_stats[i]->inode); j < max_len[0]; j++) printf(" ");
                printf("%ld ", file_stats[i]->inode);
            }

            printf("%s ", file_stats[i]->permission_string);

            for (size_t j = no_of_digits(file_stats[i]->links); j < max_len[1]; j++) printf(" ");
            printf("%ld ", file_stats[i]->links);

            for (size_t j = strlen(file_stats[i]->username); j < max_len[2]; j++) printf(" ");
            printf("%s ", file_stats[i]->username);

            for (size_t j = strlen(file_stats[i]->groupname); j < max_len[3]; j++) printf(" ");
            printf("%s ", file_stats[i]->groupname);

            for (size_t j = no_of_digits(file_stats[i]->size); j < max_len[4]; j++) printf(" ");
            printf("%ld %s %s\n", 
                file_stats[i]->size,
                file_stats[i]->last_modification,
                file_stats[i]->filename
            );

            free(file_stats[i]);
        }
        free(file_stats);
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