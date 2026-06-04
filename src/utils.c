#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "utils.h"

FileStats *get_file_stats(const char *dir_path, struct dirent *entry, bool l_flag) {
    FileStats *stats;               // Struct for storing the final formatted file stats
    char *pathname;                 // Pathname of the directory or file that we are printing
    struct stat file_stat;          // Struct for storing file stats
    struct passwd *pwd;             // Struct containing user information of the owner of the file
    struct group *grp;              // Struct containing group information of the group of users using the file
    struct tm *last_modification;   // Struct representing the time of last modification

    // Allocate memory for the stats struct
    stats = (FileStats *) malloc(sizeof(FileStats));
    if (stats == NULL) return NULL;

    // Enter the inode number
    stats->inode = entry->d_ino;

    // Enter  the name of the file
    snprintf(stats->filename, NAME_MAX+1, "%s", entry->d_name);

    // If l flag is not set, then we'll just need the inode number and filename
    if (l_flag == false) return stats;

    // Get the full pathname
    pathname = get_pathname(dir_path, entry->d_name);

    // If the pathname is NULL, return NULL
    if (pathname == NULL) return NULL;

    // Obtain the file stats
    if (lstat(pathname, &file_stat) == -1) return NULL;

    // Check if the file is a symlink or not
    if ((stats->is_link = S_ISLNK(file_stat.st_mode))) {
        if (file_stat.st_size > 0)
            stats->link_target = read_link_target(pathname, file_stat.st_size);
        else
            stats->link_target = read_link_target(pathname, PATH_MAX);
    }

    // Free the dynamic buffer for storing the pathname
    free(pathname);

    // Enter the number of blocks allocated
    stats->blocks = file_stat.st_blocks;

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

    return stats;
}

char *get_pathname(const char *dir_path, const char *filename) {
    // Make the buffer static so that the memory persists for the lifetime of the program
    // static char pathname[PATH_MAX];
    char *pathname = (char *) calloc(strlen(dir_path) + strlen(filename) + 2, sizeof(char));
    if (pathname == NULL) return NULL;
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

char *read_link_target(const char *pathname, size_t size) {
    char *path_buffer = (char *) calloc(size+1, sizeof(char));
    if (path_buffer == NULL) return NULL;

    if (readlink(pathname, path_buffer, size) == -1) {
        free(path_buffer);
        return NULL;
    }

    path_buffer[size] = '\0';
    return path_buffer;
}

size_t no_of_digits(unsigned long n) {
    size_t size = 1;

    while (n % 10 != n) {
        n /= 10;
        size++;
    }
    return size;
}

int compare_file_stats(const void *a, const void *b) {
    if (a == NULL) return -1;
    if (b == NULL) return 1;
    const FileStats **f1 = (const FileStats **) a;
    const FileStats **f2 = (const FileStats **) b;
    const char *s1 = &((*f1)->filename[0]);
    const char *s2 = &((*f2)->filename[0]);
    if (s1[0] == '.') s1 = &s1[1];
    if (s2[0] == '.') s2 = &s2[1];

    return strcoll(s1, s2);
}