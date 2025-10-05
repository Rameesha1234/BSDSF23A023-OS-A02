# REPORT

I am Rameesha Shakeel (BSDSF23A023). This report will contain answers for each feature.

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
