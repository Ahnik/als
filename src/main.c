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
#include <locale.h>
#include "utils.h"

int main(int argc, char **argv) {
    // Use the locale of the user's program
    setlocale(LC_COLLATE, "");

    // Variables used
    DIR *directory;
    struct dirent *entry;
    const char *dir_path = ".";
    int opt;
    size_t size = 0;
    size_t total_blocks = 0;
    size_t max_len[NUM_VARIABLE_FIELDS] = {0};
    FileStats **file_stats = NULL;
    bool flags[NUM_FLAGS] = {false};

    // Parse the input flags
    while ((opt = getopt(argc, argv, "ahilog")) != -1) {
        switch (opt) {
            case 'a':
                flags[A_FLAG] = true;
                break;
            case 'l':
                flags[L_FLAG] = true;
                break;
            case 'i':
                flags[I_FLAG] = true;
                break;
            case 'h':
                flags[H_FLAG] = true;
                break;
            case 'o':
                flags[O_FLAG] = true;
                break;
            case 'g':
                flags[G_FLAG] = true;
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    // If -h argument is used, simply print the help page
    if (flags[H_FLAG]) {
        print_help();
        return 0;
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
        if (flags[A_FLAG] || (entry->d_name[0] != '.')) {
            size++;
            // Check whether size of the memory length would cause an overflow
            if (size > SIZE_MAX / sizeof(FileStats *)) {
                fprintf(stderr, "%s: memory size length overflow!\n", argv[0]);
                for (size_t i = 0; i < size-1; i++) free(file_stats[i]);
                free(file_stats);
                closedir(directory);
                return 1;
            }

            // Resize the buffer for storing file metadata
            file_stats = (FileStats **) realloc(file_stats, size * sizeof(FileStats *));
            if (file_stats == NULL) {
                perror(argv[0]);
                closedir(directory);
                return 1;
            }

            // Get all the file metadata
            file_stats[size-1] = get_file_stats(dir_path, entry, flags[L_FLAG] || flags[O_FLAG] || flags[G_FLAG]);

            // If there has been any problem in fetching file stats, exit
            if (file_stats[size-1] == NULL) {
                perror(argv[0]);
                for (size_t i = 0; i < size-1; i++) free(file_stats[i]);
                free(file_stats);
                closedir(directory);
                return 1;
            }

            if (flags[L_FLAG] || flags[O_FLAG] || flags[G_FLAG]) {
                // Compare the inode number, links number, username, groupname and size of the file
                if (no_of_digits(file_stats[size-1]->inode) > max_len[0])
                    max_len[0] = no_of_digits(file_stats[size-1]->inode);
                if (no_of_digits(file_stats[size-1]->links) > max_len[1])
                    max_len[1] = no_of_digits(file_stats[size-1]->links);
                if (strlen(file_stats[size-1]->username) > max_len[2])
                    max_len[2] = strlen(file_stats[size-1]->username);
                if (strlen(file_stats[size-1]->groupname) > max_len[3])
                    max_len[3] = strlen(file_stats[size-1]->groupname);
                if (no_of_digits(file_stats[size-1]->size) > max_len[4])
                    max_len[4] = no_of_digits(file_stats[size-1]->size);

                // Add up the number of blocks allocated
                total_blocks += file_stats[size-1]->blocks;
            }

            if (flags[A_FLAG]) {
                // If the entry name is '.', then put it at the top
                if (size > 1 && (strncmp(file_stats[size-1]->filename, ".", sizeof(".")) == 0))
                    SWAP(FileStats *, file_stats, size-1, 0);
                // If the array is of size 2 and the first entry had name "..", then swap_file_stats the first two entries
                if (size == 2 && (strncmp(file_stats[0]->filename, "..", sizeof("..")) == 0))
                    SWAP(FileStats *, file_stats, 0, 1);
                // If the array is of size greater than 2 and the entry name is "..", place the entry at the second place in the array
                if (size > 2 && (strncmp(file_stats[size-1]->filename, "..", sizeof("..")) == 0))
                    SWAP(FileStats *, file_stats, size-1, 1);
            }
        }
    }

    // Sort the array of filestats by their file_entries
    if (flags[A_FLAG]) {
        if (size > 2) qsort(&file_stats[2], size-2, sizeof(FileStats *), &compare_file_stats);
    } else
        qsort(file_stats, size, sizeof(FileStats *), &compare_file_stats);

    // Print all the data about each file
    if (flags[L_FLAG] || flags[O_FLAG] || flags[G_FLAG]) {
        // Print the total number of blocks allocated to all files in the directory
        total_blocks = total_blocks >> 1;       // ls command gives 1024-byte blocks while struct stat reports 512-byte blocks
        printf("total %ld\n", total_blocks);

        for (size_t i = 0; i < size; i++) {
            if (flags[I_FLAG]) {
                printf("%*ld ", (int) max_len[0], file_stats[i]->inode);
            }

            printf("%s ", file_stats[i]->permission_string);

            printf("%*ld ", (int) max_len[1], file_stats[i]->links);

            if (!flags[G_FLAG])
                printf("%*s ", (int) max_len[2], file_stats[i]->username);

            if (!flags[O_FLAG])
                printf("%*s ", (int) max_len[3], file_stats[i]->groupname);

            printf("%*ld ", (int) max_len[4], file_stats[i]->size);
            printf("%s ", file_stats[i]->last_modification);

            if (check_for_spaces(file_stats[i]->filename, strlen(file_stats[i]->filename))) printf("'%s'", file_stats[i]->filename);
            else printf("%s", file_stats[i]->filename);

            if (file_stats[i]->permission_string[0] == 'l') {
                printf(" -> ");
                if (file_stats[i]->link_target != NULL) {
                    printf("%s", file_stats[i]->link_target);
                    free(file_stats[i]->link_target);
                }
                else
                    printf("???");
            }

            printf("\n");

            free(file_stats[i]);
        }
        free(file_stats);
    } else {
        int terminal_width = get_terminal_width();
        int rows = calc_rows(size, terminal_width, file_stats, flags[I_FLAG]);
        
        print_files(size, rows, file_stats, flags[I_FLAG]);

        for (size_t i = 0; i < size; i++) free(file_stats[i]);
        free(file_stats);
    }

    // Close the directory stream
    closedir(directory);

    if (errno != 0) {
        perror(argv[0]);
        return 1;
    }

    return 0;
}