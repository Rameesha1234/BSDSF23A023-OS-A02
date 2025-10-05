#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    const char *dir = (argc > 1) ? argv[1] : ".";
    DIR *d = opendir(dir);
    if (!d) {
        fprintf(stderr, "Cannot open '%s': %s\n", dir, strerror(errno));
        return 1;
    }
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden files
        printf("%s\n", entry->d_name);
    }
    closedir(d);
    return 0;
}
