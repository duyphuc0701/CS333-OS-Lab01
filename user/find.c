#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// Function to extract the base name (file or directory name) from a given path
char * fmtname(char* path) {
	char *p;
	for (p = path + strlen(path); p >= path && *p!='/'; p--);
	p++;

	return p;
}

void find(char *path , char *file) {
    char buf[512];
    char *p;
    int fd;
    struct dirent de;
	struct stat st;

    // Open the current directory
	if ((fd = open(path, O_RDONLY)) < 0) {
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}

    // Get the metadata of the current file/directory
	if (fstat(fd, &st) < 0) {
		fprintf(2, "find: cannot fstat %s\n", path);
        close(fd);
        return;
	}

	switch (st.type) {
        case T_DEVICE:
        case T_FILE:
            // Compare the file name with the target file name
            if (strcmp(fmtname(path), fmtname(file)) == 0) 
                fprintf(1, "%s\n", path);
            break;
        case T_DIR:
            // Check if the buffer size can accommodate the new path
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
                fprintf(2, "find: path too long\n");
                break;
            }
            // Copy the current directory path to the buffer
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            // Read entries in the directory
            while(read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0) 
                    continue;
                // Skip "." and ".." to avoid infinite loops
                if (strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
                    memmove(p, de.name, DIRSIZ);
                    // Null-terminate the path
                    p[DIRSIZ] = '\0';
                    // Recursively call find for the new path
                    find(buf, file);
                }
            }
            break;
	}
	close(fd);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(2, "The correct format should be: find [DIRECTORY_TREE_ROOT] FILE_NAME\n");
		exit(1);
	}
	if (argc < 3) {
        find(".", argv[1]);
    }
	else {
		for(int i = 1; i < argc - 1; i++) {
            find(argv[i], argv[argc - 1]);
        }
	}

	exit(0);
}