#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Function to extract the base name (file or directory name) from a given path
char *fmtname(char *path) {
    char *p;
    for (p = path + strlen(path); p >= path && *p != '/'; p--);
    return p + 1;
}

// Recursive function to find the file
void find(char *path, char *file) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // Open the directory
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // Get the metadata of the path
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
    case T_FILE:
        if (strcmp(fmtname(path), file) == 0) {
            printf("%s\n", path);
        }
        break;

    case T_DIR:
        // Ensure the buffer has enough space for the new path
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            fprintf(2, "find: path too long\n");
            break;
        }

        // Copy the directory path to the buffer
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        // Read directory entries
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;

            // Skip "." and ".." to avoid infinite loops
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            // Append the entry name to the path
            memmove(p, de.name, strlen(de.name));
            p[strlen(de.name)] = '\0';

            // Recursively call find for the new path
            find(buf, file);
        }
        break;
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: find [directory] filename\n");
        exit(1);
    }

    if (argc == 2) {
        find(".", argv[1]); // Default to current directory if no directory is provided
    } else {
        for (int i = 1; i < argc - 1; i++) {
            find(argv[i], argv[argc - 1]); // Search in specified directories
        }
    }

    exit(0);
}