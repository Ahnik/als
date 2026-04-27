#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    DIR *directory = opendir(".");

    if (directory == NULL) {
        perror("Error opening directory");
        return EXIT_FAILURE;
    } else {
        printf("The directory has been successfully openened!\n");
    }

    closedir(directory);
    return EXIT_SUCCESS;
}