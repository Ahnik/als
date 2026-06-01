#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "utils.h"

FileStats *get_file_stats(const char *dir_path, struct dirent *entry) {
    FileStats *stats;               // Struct for storing the final formatted file stats
    const char *pathname;           // Pathname of the directory or file that we are printing
    struct stat file_stat;          // Struct for storing file stats
    struct passwd *pwd;             // Struct containing user information of the owner of the file
    struct group *grp;              // Struct containing group information of the group of users using the file
    struct tm *last_modification;   // Struct representing the time of last modification

    // Allocate memory for the stats struct
    stats = (FileStats *) malloc(sizeof(FileStats));
    if (stats == NULL) return NULL;

    // Get the full pathname
    pathname = get_pathname(dir_path, entry->d_name);

    // Obtain the file stats
    if (lstat(pathname, &file_stat) == -1) {
        perror("Error while obtaining file stats");
        exit(1);
    }

    // Enter the inode number
    stats->inode = entry->d_ino;

    // Enter the password string
    stats->permission_string = get_permission(file_stat.st_mode);

    // Enter the number of links of the file
    stats->links = file_stat.st_nlink;

    // Enter the username of the file owner
    pwd = getpwuid(file_stat.st_uid);
    if (pwd == NULL)
        snprintf(stats->username, LOGIN_NAME_MAX, "%d", file_stat.st_uid);
    else
        snprintf(stats->username, LOGIN_NAME_MAX, "%s", pwd->pw_name);

    // Enter the group name of the group of users using the file
    grp = getgrgid(file_stat.st_gid);
    if (grp == NULL)
        snprintf(stats->groupname, LOGIN_NAME_MAX, "%d", file_stat.st_gid);
    else
        snprintf(stats->groupname, LOGIN_NAME_MAX, "%s", grp->gr_name);

    // Enter the size of the file
    stats->size = file_stat.st_size;

    // Enter the time of last modification in %b %e %H:%M format
    last_modification = localtime(&file_stat.st_mtim.tv_sec);
    if (strftime(stats->last_modification, TIMESTAMP_SIZE, "%b %e %H:%M", last_modification) == 0)
        snprintf(stats->last_modification, TIMESTAMP_SIZE, "???         ");

    // Enter  the name of the file
    stats->filename = entry->d_name;
    return stats;
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

size_t no_of_digits(unsigned long n) {
    size_t size = 1;

    while (n % 10 != n) {
        n /= 10;
        size++;
    }
    return size;
}