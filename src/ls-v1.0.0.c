/* Step 3: Full long-listing formatting (permissions, owner, group, size, time) */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdint.h>

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

/* Convert mode to rwxrwxrwx string with file type */
void mode_to_str(mode_t m, char *buf) {
    buf[0] = S_ISDIR(m) ? 'd' :
             S_ISLNK(m) ? 'l' :
             S_ISCHR(m) ? 'c' :
             S_ISBLK(m) ? 'b' :
             S_ISFIFO(m)? 'p' :
             S_ISSOCK(m)? 's' : '-';
    buf[1] = (m & S_IRUSR) ? 'r' : '-';
    buf[2] = (m & S_IWUSR) ? 'w' : '-';
    buf[3] = (m & S_IXUSR) ? ((m & S_ISUID) ? 's' : 'x') : ((m & S_ISUID) ? 'S' : '-');
    buf[4] = (m & S_IRGRP) ? 'r' : '-';
    buf[5] = (m & S_IWGRP) ? 'w' : '-';
    buf[6] = (m & S_IXGRP) ? ((m & S_ISGID) ? 's' : 'x') : ((m & S_ISGID) ? 'S' : '-');
    buf[7] = (m & S_IROTH) ? 'r' : '-';
    buf[8] = (m & S_IWOTH) ? 'w' : '-';
    buf[9] = (m & S_IXOTH) ? 'x' : '-';
    buf[10] = '\0';
}

/* Simple list (unchanged) */
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

/* Long listing implementation */
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
            memset(&arr[n].st, 0, sizeof(struct stat));
        }
        n++;
    }
    closedir(d);

    /* compute column widths */
    int max_links = 0, max_owner = 0, max_group = 0, max_size = 0;
    for (size_t i = 0; i < n; ++i) {
        char tmp[128];
        snprintf(tmp, sizeof(tmp), "%lu", (unsigned long)arr[i].st.st_nlink);
        if ((int)strlen(tmp) > max_links) max_links = strlen(tmp);

        struct passwd *pw = getpwuid(arr[i].st.st_uid);
        struct group *gr = getgrgid(arr[i].st.st_gid);
        const char *owner = pw ? pw->pw_name : tmp; /* fallback */
        const char *group = gr ? gr->gr_name : "";
        if ((int)strlen(owner) > max_owner) max_owner = strlen(owner);
        if ((int)strlen(group) > max_group) max_group = strlen(group);

        snprintf(tmp, sizeof(tmp), "%lld", (long long)arr[i].st.st_size);
        if ((int)strlen(tmp) > max_size) max_size = strlen(tmp);
    }

    /* print each entry in long format */
    for (size_t i = 0; i < n; ++i) {
        char perm[11];
        mode_to_str(arr[i].st.st_mode, perm);

        struct passwd *pw = getpwuid(arr[i].st.st_uid);
        struct group *gr = getgrgid(arr[i].st.st_gid);
        char ownerbuf[64] = {0}, groupbuf[64] = {0};
        if (pw) strncpy(ownerbuf, pw->pw_name, sizeof(ownerbuf)-1);
        else snprintf(ownerbuf, sizeof(ownerbuf), "%u", arr[i].st.st_uid);
        if (gr) strncpy(groupbuf, gr->gr_name, sizeof(groupbuf)-1);
        else snprintf(groupbuf, sizeof(groupbuf), "%u", arr[i].st.st_gid);

        /* time formatting: "Mon DD HH:MM" */
        char timebuf[64] = {0};
        struct tm *tm = localtime(&arr[i].st.st_mtime);
        if (tm) strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

        /* name (handle symlink target) */
        char namebuf[PATH_MAX+128];
        if (S_ISLNK(arr[i].st.st_mode)) {
            char linktarget[PATH_MAX];
            ssize_t r = readlink(arr[i].path, linktarget, sizeof(linktarget)-1);
            if (r >= 0) { linktarget[r] = '\0'; snprintf(namebuf, sizeof(namebuf), "%s -> %s", arr[i].name, linktarget); }
            else snprintf(namebuf, sizeof(namebuf), "%s", arr[i].name);
        } else {
            snprintf(namebuf, sizeof(namebuf), "%s", arr[i].name);
        }

        printf("%s %*lu %-*s %-*s %*lld %s %s\n",
               perm,
               max_links, (unsigned long)arr[i].st.st_nlink,
               max_owner, ownerbuf,
               max_group, groupbuf,
               max_size, (long long)arr[i].st.st_size,
               timebuf,
               namebuf);
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
