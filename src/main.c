#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pwd.h>
#include <grp.h>

// Maximum length for a complete pathname
#define PATH_MAX 4096

// Size of the permission string
#define PERMISSION_STRING_SIZE 11

// Function to print the long version of a directory entry
void print_long_list(const char *pathname);

// Function to extract read, write and execute permissions of the owner, group and others
const char *get_permission(mode_t file_mode);

// Function to join the directory name and the filename together to form pathname
const char *get_pathname(const char *dir_path, const char *filename);

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
    struct stat file_stat;          // Struct for storing file stats
    const char *file_permission;    // File permission string
    struct passwd *pwd;             // Struct containing user information of the owner of the file
    struct group *grp;              // Struct containing group information of the group of users using the file

    // Obtain the file stats
    if (lstat(pathname, &file_stat) == -1) {
        perror("Error while obtaining file stats");
        exit(1);
    }

    /* TODO: Write logic to print the long version of ls of a file entry */
    // Write the password string
    file_permission = get_permission(file_stat.st_mode);
    printf("%s ", file_permission);

    // Write the number of links of the file
    printf("%ld ", file_stat.st_nlink);

    // Write the username of the file owner
    pwd = getpwuid(file_stat.st_uid);
    printf("%s ", pwd->pw_name);

    // Write the group name of the group of users using the file
    grp = getgrgid(file_stat.st_gid);
    printf("%s ", grp->gr_name);
    printf("\n");
}

const char *get_pathname(const char *dir_path, const char *filename) {
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

const char *get_permission(mode_t file_mode) {
    // Declare the static buffer for the permission string
    static char file_permission[PERMISSION_STRING_SIZE];

    // Set the entire permission string with hyphens by default
    for (int i = 0; i < PERMISSION_STRING_SIZE-1; i++) {
        file_permission[i] = '-';
    }
    file_permission[PERMISSION_STRING_SIZE-1] = 0;

    // Check if the file is a directory or not
    if (S_ISDIR(file_mode))
        file_permission[0] = 'd';

    // Check for read permission of the owner
    if ((file_mode & S_IRUSR) == S_IRUSR)
        file_permission[1] = 'r';

    // Check for write permission of the owner
    if ((file_mode & S_IWUSR) == S_IWUSR)
        file_permission[2] = 'w';

    // Check for execute permission of the owner
    if ((file_mode & S_IXUSR) == S_IXUSR)
        file_permission[3] = 'x';

    // Check for read permission of the group
    if ((file_mode & S_IRGRP) == S_IRGRP)
        file_permission[4] = 'r';

    // Check for write permission of the group
    if ((file_mode & S_IWGRP) == S_IWGRP)
        file_permission[5] = 'w';

    // Check for execute permission of the group
    if ((file_mode & S_IXGRP) == S_IXGRP)
        file_permission[6] = 'x';

    // Check for read permission of others
    if ((file_mode & S_IROTH) == S_IROTH)
        file_permission[7] = 'r';

    // Check for write permission of others
    if ((file_mode & S_IWOTH) == S_IWOTH)
        file_permission[8] = 'w';

    // Check for execute permission of others
    if ((file_mode & S_IXOTH) == S_IXOTH)
        file_permission[9] = 'x';

    return file_permission;
}