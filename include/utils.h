#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

// Maximum length for a complete pathname
#ifndef PATH_MAX
#ifdef _POSIX_PATH_MAX
    #define PATH_MAX (_POSIX_PATH_MAX)
#else
    #define PATH_MAX 4096
#endif
#endif

// Maximum length for a filename
#ifndef NAME_MAX
#ifdef _POSIX_NAME_MAX
    #define NAME_MAX (_POSIX_NAME_MAX)
#else
    #define NAME_MAX 256
#endif
#endif

// Maximum length for a username or groupname
#ifndef LOGIN_NAME_MAX
#ifdef _POSIX_LOGIN_NAME_MAX
    #define LOGIN_NAME_MAX (_POSIX_LOGIN_NAME_MAX)
#else
    #define LOGIN_NAME_MAX 256
#endif
#endif

// Size of the permission string
#define PERMISSION_STRING_SIZE 11

// Size of the timestamp string
#define TIMESTAMP_SIZE 13

// Number of variable length fields
#define NUM_VARIABLE_FIELDS 5

// Struct to store the fields to be printed in the long list
typedef struct {
    ino_t inode;                             // inode number
    const char *permission_string;           // permission string
    nlink_t links;                           // number of links
    char username[LOGIN_NAME_MAX];           // file owner name
    char groupname[LOGIN_NAME_MAX];          // file group owner name
    off_t size;                              // file size
    char last_modification[TIMESTAMP_SIZE];  // time of last modification
    const char *filename;                    // name of the file
} FileStats;

// Function to get the file stats of a file
FileStats *get_file_stats(const char *dir_path, struct dirent *entry);

// Function to extract read, write and execute permissions of the owner, group and others
const char *get_permission(mode_t file_mode);

// Function to join the directory name and the filename together to form pathname
const char *get_pathname(const char *dir_path, const char *filename);

// Function to get the number of digits in an integer
size_t no_of_digits(unsigned long n);

#endif