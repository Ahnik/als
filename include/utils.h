#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

// Maximum length for a pathname
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
typedef struct _FileStats {
    __blkcnt_t blocks;                                        // number of blocks allocated to this file
    ino_t inode;                                              // inode number
    char permission_string[PERMISSION_STRING_SIZE];           // permission string
    nlink_t links;                                            // number of links
    char username[LOGIN_NAME_MAX];                            // file owner name
    char groupname[LOGIN_NAME_MAX];                           // file group owner name
    off_t size;                                               // file size
    char *link_target;                                        // target of the link if the file is a symlink
    char last_modification[TIMESTAMP_SIZE];                   // time of last modification
    char filename[NAME_MAX+1];                                // name of the file
} FileStats;

// A macro function to swap two elements in an array
#define SWAP(type, arr, a, b) \
    do { \
        type temp = arr[a]; \
        arr[a] = arr[b]; \
        arr[b] = temp; \
    } while (0)

// Function to get the file stats of a file
FileStats *get_file_stats(const char *dir_path, struct dirent *entry, bool l_flag);

// Function to extract read, write and execute permissions of the owner, group and others
void get_permission(mode_t file_mode, char *file_permission);

// Function to join the directory name and the filename together to form pathname
char *get_pathname(const char *dir_path, const char *filename);

// Function to read the link target if the file is a symlink
char *read_link_target(const char *pathname, size_t size);

// Function to get the number of digits in an integer
size_t no_of_digits(unsigned long n);

// Comparator to compare two filestats objects for sorting according to their filenames
int compare_file_stats(const void *a, const void *b);

// Function to check for spaces
bool check_for_spaces(const char *filename, size_t size);

// Print help page
void print_help();

// Function to get terminal width
int get_terminal_width();

// Function to calculate display length of the file entry
int get_display_length(FileStats *stat, bool i_flag);

// Function to calculate optimal number of rows for multi-column layout
int calc_rows(int size, int terminal_width, FileStats **file_stats, bool i_flag);

// Function to calculate total width needed for a given number of rows
int calc_width(int rows, int size, FileStats **file_stats, bool i_flag);

// Function to print files in multi-column format
void print_files(int size, int rows, FileStats **file_stats, bool i_flag);

#endif