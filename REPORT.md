# REPORT

I am Rameesha Shakeel (BSDSF23A023). This report will contain answers for each feature.

## Feature-1: Project Setup and Initial Build
*(No report questions were assigned for this feature.)*

---
## Feature-2 Report

**Q1: What is the crucial difference between stat() and lstat()?  
When is it more appropriate to use lstat()?**

- `stat()` follows symbolic links and returns information about the file that the link points to.  
- `lstat()` does not follow symbolic links and instead returns information about the link itself.  
- In `ls`, `lstat()` is more appropriate because we want to correctly identify symbolic links (show them as `l` in the permission string).

---

**Q2: How to extract file type and permission bits from st_mode?**

- `st_mode` is a bit field that encodes both file type and permission bits.  
- File type is extracted using macros like `S_ISDIR(st_mode)` (directory), `S_ISREG(st_mode)` (regular file), etc.  
- Permissions are checked with bitwise AND:  
  - `st_mode & S_IRUSR` → owner read permission  
  - `st_mode & S_IWUSR` → owner write permission  
  - `st_mode & S_IXUSR` → owner execute permission  
- Similarly for group (`S_IRGRP`, etc.) and others (`S_IROTH`, etc.).

## Feature-3: Column Display (Down Then Across)

**Q1. Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop insufficient?**

- In "down then across" layout, filenames are printed vertically first, then move across columns.  
- The program calculates how many rows and columns fit based on terminal width and longest filename length.  
- For each row, it prints items at indexes:  
  - row  
  - row + rows  
  - row + 2*rows  
  - … and so on, until all columns are printed.  
- A single sequential loop would only produce horizontal (row-major) output, not the required vertical layout.

---

**Q2. What is the purpose of the ioctl system call in this context? What are the limitations of using a fixed width (e.g., 80 columns)?**

- `ioctl()` with the `TIOCGWINSZ` request returns the current terminal window size (number of columns).  
- This allows the `ls` program to dynamically adapt its column layout to the user’s screen width.  
- If only a fixed width (like 80 characters) is assumed:  
  - On wider terminals, output would waste space and look sparse.  
  - On narrower terminals, filenames could wrap incorrectly or misalign.
 
  - Feature-4: Long Listing (-l Option)

Q1. What is the purpose of the -l option in ls, and what new information does it display?

The -l option enables long-listing mode, which displays detailed information about each file or directory line by line instead of the default multi-column format.

It provides additional details, including:

File type and permissions (e.g., -rw-r--r--)

Number of hard links

Owner (username)

Group name

File size (in bytes)

Last modification time

File name

This behavior matches the standard Linux ls -l command format.

Q2. Which system calls or library functions are used to implement the long-listing mode, and what are their purposes?

stat() → Retrieves file metadata such as permissions, size, ownership, and timestamps.

getpwuid() → Converts the numeric user ID (UID) into the corresponding username.

getgrgid() → Converts the numeric group ID (GID) into the group name.

localtime() → Converts the file’s last modification time into a local time structure.

strftime() → Formats the time into a readable date/time string (e.g., Oct 5 09:40).

Q3. Describe the major code modifications required for this feature.

Added a long_flag variable and updated command-line argument parsing in main() to detect the -l option.

Modified the do_ls() function signature to do_ls(const char *dir, int long_flag) to support both normal and long-listing outputs.

Implemented a helper function print_long() that uses stat() to print detailed file information in formatted style.

Preserved all previous functionalities:

Hidden files remain excluded (Feature-2).

Files stay alphabetically sorted.

Default output still uses the column layout (Feature-3) when -l is not specified.

Q4. How was this feature tested, and what were the results?

Commands tested:

./bin/ls
./bin/ls -l
./bin/ls -l /etc


Results:

./bin/ls → Displays files in multi-column layout (Feature-3).

./bin/ls -l → Displays one file per line with full details.

Works correctly for multiple directories.

Hidden files remain excluded, as per previous features.

✅ Conclusion:
Feature-4 successfully implements the long-listing (-l) option, enhancing the program to display file permissions, ownership, size, and modification time.
All previous features remain functional, making the output format similar to the standard Unix ls -l behavior.

- Therefore, `ioctl()` makes the program more robust and professional.

