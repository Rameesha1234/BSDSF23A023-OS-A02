/* Step 1: Add -l parsing and stubs */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void print_simple_list(const char *dir) {
    DIR *d = opendir(dir);
    if (!d) {
        fprintf(stderr, "Cannot open '%s': %s\n", dir, strerror(errno));
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        printf("%s\n", entry->d_name);
    }
    closedir(d);
}

void print_long_list(const char *dir) {
    /* stub: will be implemented in later commits */
    printf("Long-listing (stub) for directory: %s\n", dir);
    DIR *d = opendir(dir);
    if (!d) { perror("opendir"); return; }
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        printf("%s\n", entry->d_name);
    }
    closedir(d);
}

int main(int argc, char *argv[]) {
    int opt;
    int long_flag = 0;

    /* parse options: only -l for now */
    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l': long_flag = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
                return 1;
        }
    }

    const char *dir = (optind < argc) ? argv[optind] : ".";

    if (long_flag) print_long_list(dir);
    else print_simple_list(dir);

    return 0;
}
