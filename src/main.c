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

    // Flags used
    bool a_flag = false;
    bool l_flag = false;
    bool i_flag = false;
    bool h_flag = false;

    // Parse the input flags
    while ((opt = getopt(argc, argv, "ahil")) != -1) {
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
            case 'h':
                h_flag = true;
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    // If -h argument is used, simply print the help page
    if (h_flag) {
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
        if (a_flag || (entry->d_name[0] != '.')) {
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
            file_stats[size-1] = get_file_stats(dir_path, entry, l_flag);

            // If there has been any problem in fetching file stats, exit
            if (file_stats[size-1] == NULL) {
                perror(argv[0]);
                for (size_t i = 0; i < size-1; i++) free(file_stats[i]);
                free(file_stats);
                closedir(directory);
                return 1;
            }

            if (l_flag) {
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

            if (a_flag) {
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
    if (a_flag) {
        if (size > 2) qsort(&file_stats[2], size-2, sizeof(FileStats *), &compare_file_stats);
    } else
        qsort(file_stats, size, sizeof(FileStats *), &compare_file_stats);

    // Print all the data about each file
    if (l_flag) {
        // Print the total number of blocks allocated to all files in the directory
        total_blocks = total_blocks >> 1;       // ls command gives 1024-byte blocks while struct stat reports 512-byte blocks
        printf("total %ld\n", total_blocks);

        for (size_t i = 0; i < size; i++) {
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
            printf("%ld %s ", 
                file_stats[i]->size,
                file_stats[i]->last_modification
            );

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
        int rows = calc_rows(size, terminal_width, file_stats, i_flag);
        
        print_files(size, rows, file_stats, i_flag);

        for (size_t i = 0; i < size; ++i) free(file_stats[i]);
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