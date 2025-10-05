/* Step 2: Read entries into memory and collect lstat info */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

typedef struct {
    char *name;
    char *path;
    struct stat st;
} FileEntry;

void free_entries(FileEntry *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        free(arr[i].name);
        free(arr[i].path);
    }
    free(arr);
}

int num_digits_longlong(long long x) {
    char tmp[64];
    return snprintf(tmp, sizeof(tmp), "%lld", x);
}

void print_simple_list(const char *dir) {
    DIR *d = opendir(dir);
    if (!d) { perror("opendir"); return; }
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        printf("%s\n", entry->d_name);
    }
    closedir(d);
}

/* New: gather entries and lstat them */
void print_long_list(const char *dir) {
    DIR *d = opendir(dir);
    if (!d) { perror("opendir"); return; }

    FileEntry *arr = NULL;
    size_t n = 0, cap = 0;
    struct dirent *entry;

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        if (n + 1 > cap) {
            cap = cap ? cap * 2 : 64;
            arr = realloc(arr, cap * sizeof(FileEntry));
            if (!arr) { perror("realloc"); closedir(d); return; }
        }

        arr[n].name = strdup(entry->d_name);
        arr[n].path = malloc(PATH_MAX);
        if (!arr[n].name || !arr[n].path) { perror("malloc/strdup"); free_entries(arr, n); closedir(d); return; }

        if (strcmp(dir, ".") == 0) snprintf(arr[n].path, PATH_MAX, "%s", entry->d_name);
        else snprintf(arr[n].path, PATH_MAX, "%s/%s", dir, entry->d_name);

        if (lstat(arr[n].path, &arr[n].st) != 0) {
            /* if lstat fails, zero the struct */
            memset(&arr[n].st, 0, sizeof(struct stat));
        }
        n++;
    }
    closedir(d);

    /* compute some basic widths so printing later can align columns */
    int max_links = 0, max_size = 0;
    for (size_t i = 0; i < n; ++i) {
        int len_links = num_digits_longlong((long long)arr[i].st.st_nlink);
        if (len_links > max_links) max_links = len_links;
        int len_size = num_digits_longlong((long long)arr[i].st.st_size);
        if (len_size > max_size) max_size = len_size;
    }

    /* temporary simple output showing we collected stats */
    for (size_t i = 0; i < n; ++i) {
        printf("%*lu %*lld %s\n",
               max_links, (unsigned long)arr[i].st.st_nlink,
               max_size, (long long)arr[i].st.st_size,
               arr[i].name);
    }

    free_entries(arr, n);
}

int main(int argc, char *argv[]) {
    int opt;
    int long_flag = 0;
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
