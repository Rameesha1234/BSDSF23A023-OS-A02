/*
* Programming Assignment 02: lsv1.0.0
* Author: Rameesha Shakeel (BSDSF23A023)
* Features implemented:Feature 1–7 (-a, -l, -R, -t)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

extern int errno;

void do_ls(const char *dir, int long_flag, int all_flag);
void print_columns(char *names[], int count);
void print_long(const char *dir, const char *filename);

int main(int argc, char const *argv[])
{
    int long_flag = 0;
    int all_flag = 0;
    const char *dirs[64];
    int dir_count = 0;

    /* Parse arguments: handle -l, -a, -la, -al; don't add flags to dirs[] */
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0) { long_flag = 1; continue; }
        if (strcmp(argv[i], "-a") == 0) { all_flag = 1; continue; }
        if (strcmp(argv[i], "-la") == 0 || strcmp(argv[i], "-al") == 0) { long_flag = 1; all_flag = 1; continue; }
        /* ignore anything that looks like a flag */
        if (argv[i][0] == '-') continue;
        /* otherwise treat as directory */
        dirs[dir_count++] = argv[i];
    }

    if (dir_count == 0)
        do_ls(".", long_flag, all_flag);
    else {
        for (int i = 0; i < dir_count; i++) {
            printf("Directory listing of %s:\n", dirs[i]);
            do_ls(dirs[i], long_flag, all_flag);
            puts("");
        }
    }
    return 0;
}

/* Read directory entries, sort, and print (columns or long) */
void do_ls(const char *dir, int long_flag, int all_flag)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (!dp) { fprintf(stderr, "Cannot open directory: %s\n", dir); return; }

    errno = 0;
    char **names = NULL;
    int count = 0, capacity = 0;

    while ((entry = readdir(dp)) != NULL)
    {
        if (!all_flag && entry->d_name[0] == '.') continue;

        if (count >= capacity) {
            capacity = (capacity == 0) ? 64 : capacity * 2;
            char **tmp = realloc(names, capacity * sizeof(char *));
            if (!tmp) { perror("realloc"); for (int k=0;k<count;k++) free(names[k]); free(names); closedir(dp); return; }
            names = tmp;
        }

        names[count] = strdup(entry->d_name);
        if (!names[count]) { perror("strdup"); for (int k=0;k<count;k++) free(names[k]); free(names); closedir(dp); return; }
        count++;
    }

    if (errno != 0) perror("readdir failed");
    closedir(dp);

    /* simple sort */
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (strcmp(names[i], names[j]) > 0) { char *t = names[i]; names[i] = names[j]; names[j] = t; }

    if (long_flag) {
        for (int i = 0; i < count; i++) print_long(dir, names[i]);
    } else {
        print_columns(names, count);
    }

    for (int i = 0; i < count; i++) free(names[i]);
    free(names);
}

/* Column layout: down-then-across */
void print_columns(char *names[], int count)
{
    if (count == 0) return;
    int maxlen = 0;
    for (int i = 0; i < count; i++) { int l = strlen(names[i]); if (l > maxlen) maxlen = l; }

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0) w.ws_col = 80;
    int spacing = 2;
    int colw = maxlen + spacing;
    int cols = w.ws_col / colw; if (cols < 1) cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = r + c * rows;
            if (idx < count) printf("%-*s", colw, names[idx]);
        }
        printf("\n");
    }
}

/* Long format per file */
void print_long(const char *dir, const char *filename)
{
    char path[1024];
    if (strcmp(dir, ".") == 0) snprintf(path, sizeof(path), "%s", filename);
    else snprintf(path, sizeof(path), "%s/%s", dir, filename);

    struct stat st;
    if (stat(path, &st) == -1) { perror("stat"); return; }

    char perms[11];
    perms[0] = S_ISDIR(st.st_mode) ? 'd' : '-';
    perms[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    perms[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    char timebuf[64];
    struct tm *mt = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", mt);

    printf("%s %2ld %-8s %-8s %8ld %s %s\n",
           perms,
           (long)st.st_nlink,
           pw ? pw->pw_name : "",
           gr ? gr->gr_name : "",
           (long)st.st_size,
           timebuf,
           filename);
}
