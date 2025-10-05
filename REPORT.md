# REPORT

I am **Rameesha Shakeel (BSDSF23A023)**. This report contains answers for each feature of the assignment.

---

## Feature-1: Project Setup and Initial Build
*(No report questions were assigned for this feature.)*

---

## Feature-2: File Metadata and Permissions

**Q1. What is the crucial difference between `stat()` and `lstat()`? When is it more appropriate to use `lstat()`?**

- `stat()` follows symbolic links and returns information about the file that the link points to.  
- `lstat()` does not follow symbolic links and instead returns information about the link itself.  
- In `ls`, `lstat()` is more appropriate because we want to correctly identify symbolic links (show them as `l` in the permission string).

---

**Q2. How to extract file type and permission bits from `st_mode`?**

- `st_mode` is a bit field that encodes both file type and permission bits.  
- File type is extracted using macros like:  
  - `S_ISDIR(st_mode)` → directory  
  - `S_ISREG(st_mode)` → regular file  
- Permissions are checked with bitwise AND:  
  - `st_mode & S_IRUSR` → owner read permission  
  - `st_mode & S_IWUSR` → owner write permission  
  - `st_mode & S_IXUSR` → owner execute permission  
- Similarly for group (`S_IRGRP`, etc.) and others (`S_IROTH`, etc.).

---

## Feature-3: Column Display (Down Then Across)

**Q1. Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop insufficient?**

- In "down then across" layout, filenames are printed vertically first, then move across columns.  
- The program calculates how many rows and columns fit based on terminal width and longest filename length.  
- For each row, it prints items at indexes:  
  - `row`  
  - `row + rows`  
  - `row + 2*rows`  
  - … and so on, until all columns are printed.  
- A single sequential loop would only produce horizontal (row-major) output, not the required vertical layout.

---

**Q2. What is the purpose of the `ioctl()` system call in this context? What are the limitations of using a fixed width (e.g., 80 columns)?**

- `ioctl()` with the `TIOCGWINSZ` request returns the current terminal window size (number of columns).  
- This allows the `ls` program to dynamically adapt its column layout to the user’s screen width.  
- If only a fixed width (like 80 characters) is assumed:  
  - On wider terminals, output would waste space and look sparse.  
  - On narrower terminals, filenames could wrap incorrectly or misalign.

---

## Feature-4: Long Listing (-l Option)

**Q1. What is the purpose of the `-l` option in `ls`, and what new information does it display?**

- The `-l` option enables long-listing mode, which displays detailed information about each file or directory line by line instead of the default multi-column format.  
- It provides:  
  - File type and permissions (e.g., `-rw-r--r--`)  
  - Number of hard links  
  - Owner (username)  
  - Group name  
  - File size (in bytes)  
  - Last modification time  
  - File name  

---

**Q2. Which system calls or library functions are used to implement the long-listing mode, and what are their purposes?**

- `stat()` / `lstat()` → Retrieves file metadata such as permissions, size, ownership, and timestamps.  
- `getpwuid()` → Converts the numeric user ID (UID) into the corresponding username.  
- `getgrgid()` → Converts the numeric group ID (GID) into the group name.  
- `localtime()` → Converts the file’s last modification time into a local time structure.  
- `strftime()` → Formats the time into a readable string (e.g., `Oct 5 09:40`).  

---

**Q3. Describe the major code modifications required for this feature.**

- Added a `long_flag` variable and updated argument parsing in `main()` to detect the `-l` option.  
- Modified the `do_ls()` function signature to accept the long flag.  
- Implemented a helper function `print_entry()` that uses `lstat()` to print detailed file information.  
- Preserved all previous functionalities:  
  - Hidden files remain excluded unless `-a` is used.  
  - Sorting is still alphabetical.  

---

**Q4. How was this feature tested, and what were the results?**

Commands tested:
```bash
./bin/ls
./bin/ls -l
./bin/ls -l /etc

```

**Results:**
- `./bin/ls` → Multi-column layout (Feature-3).  
- `./bin/ls -l` → One file per line with full details.  
- Works for multiple directories.  
- Hidden files excluded unless combined with `-a`.  

✅ Feature-4 works as expected and matches real `ls -l`.

---

## Feature-5: Display Hidden Files (-a Flag)

**Q1. What is the purpose of the `-a` option in `ls`?**

- The `-a` option includes hidden files (those starting with `.` like `.bashrc`).  
- Without `-a`, these are skipped.

---

**Q2. How was the `-a` flag implemented?**

- In argument parsing, the program checks for `-a`, `-la`, or `-al`.  
- When set, `all_flag = 1`.  
- While reading entries with `readdir()`, normally hidden files are skipped unless `all_flag` is active:

```c
if (!all_flag && entry->d_name[0] == '.')
    continue;
Q3. Why support combined flags like -la or -al?

Real ls implementations allow combining options.

The parser detects combined forms and enables both flags.

Q4. What should the output look like?

Without -a: only visible files.

With -a: hidden files like ., .., .git also shown.

With -la: all files shown with long-format details.
```
Example:

```bash
drwxrwxr-x 2 rameesha rameesha 4096 Oct  5 10:13 .
drwxrwxr-x 5 rameesha rameesha 4096 Oct  5 09:00 ..
-rw-rw-r-- 1 rameesha rameesha  292 Oct  5 05:52 Makefile
```
## Feature-6: Recursive Directory Listing (-R Flag)
---
**Q1. What is the purpose of the -R flag in ls?

The -R option lists not only the current directory’s contents but also all subdirectories recursively.

---

**Q2. How was recursion implemented?

A new variable recursive_flag is set when -R is detected.

Inside do_ls(), for each entry that is a directory (excluding . and ..), the function calls itself recursively:

```c
if (recursive_flag && S_ISDIR(statbuf.st_mode) &&
    strcmp(names[i], ".") != 0 &&
    strcmp(names[i], "..") != 0)
{
    printf("\n%s:\n", path);
    do_ls(path, all_flag, long_flag, recursive_flag, time_flag);
}
```
---
**Q3. How do we prevent infinite recursion (e.g., with . and ..)?

The program explicitly skips these names so recursion never loops back.

---

**Q4. What is the expected output of ./bin/ls -R?

Example:

```bash
.:
Makefile  src  obj  bin

./src:
ls-v1.0.0.c

./obj:
ls-v1.0.0.o

./bin:
ls
With combined flags (-lR, -aR, etc.), the output is detailed and recursive.
```
## Feature-7: Sorting by Modification Time (-t)
---
**Q1. What does the -t flag do in ls?

It sorts files and directories by their last modification time (st_mtime), newest first.

---
**Q2. How was this implemented?

Added a time_flag.

Extended do_ls() to accept time_flag.

Implemented compare_times() using lstat() and st_mtime.

Switched to compare_times when -t is active.
---
**Q3. How are combined flags handled (like -lt, -lat)?

The parser sets multiple flags when these are used (long_flag + time_flag, etc.), matching real ls behavior.
---
**Q4. Example outputs

```bash

$ touch oldfile
$ sleep 2
$ touch newfile

$ ./bin/ls -t
newfile    oldfile

$ ./bin/ls -lt
-rw-r--r-- 1 rameesha rameesha     0 Oct  5 10:12 newfile
-rw-r--r-- 1 rameesha rameesha     0 Oct  5 10:10 oldfile

$ ./bin/ls -lat
.  ..  newfile  oldfile  Makefile  ...
```
---
## 📌 Conclusion
In this project, I successfully implemented a simplified but powerful version of the Unix ls command (lsv1.0.0).

The program now supports:

Basic file listing (Feature-1)

Correct handling of file types and permissions with lstat() (Feature-2)

Dynamic multi-column layout (Feature-3)

Long listing with detailed metadata (-l) (Feature-4)

Displaying hidden files (-a) and combined flags like -la (Feature-5)

Recursive directory traversal (-R) with safe handling of . and .. (Feature-6)

Sorting by modification time (-t), including combined options (-lt, -lat, etc.) (Feature-7)
---

## Key Takeaways
Learned how system calls (stat, lstat, readdir, ioctl) provide file metadata and terminal info.

Practiced argument parsing to support multiple flags and their combinations.

Reinforced concepts of recursion, memory management, and safe string handling (snprintf, PATH_MAX).

Produced modular, testable code that evolves feature by feature, just like professional development workflows.

Used incremental commits and clear documentation (REPORT.md) to mirror real software engineering practices.

---
## Final Result
My implementation behaves very close to the standard Linux ls, while being written entirely from scratch in C.
This assignment gave me deep insights into both UNIX internals and software engineering discipline (version control, testing, and reporting).
