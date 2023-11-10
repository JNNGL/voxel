#include <dirent.h>
#include <stdio.h>

int main(int argc, char* const argv[]) {
    char* path = "/"; // TODO: syscall_getcwd / get from env
    if (argc > 1) {
        path = argv[1];
    }

    DIR* dir = opendir(path);
    if (!dir) {
        perror("ls: cannot open directory");
        return 0;
    }

    size_t printed = 0;

    struct dirent* dirent;
    while ((dirent = readdir(dir)) != 0) {
        if (dirent->d_name[0] == '.') {
            continue;
        }
        printf("%-15s",  dirent->d_name);
        ++printed;
    }

    if (printed) {
        printf("\n");
    }

    closedir(dir);
    return 0;
}