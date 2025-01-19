#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Function to extract the base name (file or directory name) from a given path
char *extractBaseName(char *path) {
    char *lastSlash;
    for (lastSlash = path + strlen(path); lastSlash >= path && *lastSlash != '/'; lastSlash--);
    return lastSlash + 1;
}

// Recursive function to search for the file
void searchFile(char *directoryPath, char *targetFileName) {
    char pathBuffer[512], *pathPointer;
    int directoryFd;
    struct dirent directoryEntry;
    struct stat directoryStats;

    // Open the directory
    if ((directoryFd = open(directoryPath, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", directoryPath);
        return;
    }

    // Get the metadata of the directory
    if (fstat(directoryFd, &directoryStats) < 0) {
        fprintf(2, "find: cannot stat %s\n", directoryPath);
        close(directoryFd);
        return;
    }

    switch (directoryStats.type) {
    case T_FILE:
        if (strcmp(extractBaseName(directoryPath), targetFileName) == 0) {
            printf("%s\n", directoryPath);
        }
        break;

    case T_DIR:
        // Ensure the buffer has enough space for the new path
        if (strlen(directoryPath) + 1 + DIRSIZ + 1 > sizeof(pathBuffer)) {
            fprintf(2, "find: path too long\n");
            break;
        }

        // Copy the directory path to the buffer
        strcpy(pathBuffer, directoryPath);
        pathPointer = pathBuffer + strlen(pathBuffer);
        *pathPointer++ = '/';

        // Read directory entries
        while (read(directoryFd, &directoryEntry, sizeof(directoryEntry)) == sizeof(directoryEntry)) {
            if (directoryEntry.inum == 0)
                continue;

            // Skip "." and ".." to avoid infinite loops
            if (strcmp(directoryEntry.name, ".") == 0 || strcmp(directoryEntry.name, "..") == 0)
                continue;

            // Append the entry name to the path
            memmove(pathPointer, directoryEntry.name, strlen(directoryEntry.name));
            pathPointer[strlen(directoryEntry.name)] = '\0';

            // Recursively call searchFile for the new path
            searchFile(pathBuffer, targetFileName);
        }
        break;
    }

    close(directoryFd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: find [directory] filename\n");
        exit(1);
    }

    if (argc == 2) {
        searchFile(".", argv[1]); // Default to current directory if no directory is provided
    } else {
        for (int i = 1; i < argc - 1; i++) {
            searchFile(argv[i], argv[argc - 1]); // Search in specified directories
        }
    }

    exit(0);
}