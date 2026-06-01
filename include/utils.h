#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>

// Maximum length for a complete pathname
#define PATH_MAX 4096

// Size of the permission string
#define PERMISSION_STRING_SIZE 11

// Size of the timestamp string
#define TIMESTAMP_SIZE 13

// Number of variable length fields
#define NUM_VARIABLE_FIELDS 5

// Function to print the long version of a directory entry
void print_long_list(const char *dir_path, struct dirent *entry, bool i_flag);

// Function to extract read, write and execute permissions of the owner, group and others
const char *get_permission(mode_t file_mode);

// Function to join the directory name and the filename together to form pathname
const char *get_pathname(const char *dir_path, const char *filename);

#endif