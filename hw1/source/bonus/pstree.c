#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_DEPTH 128

void print_tree(int parent_pid, int indent, int depth);

int main() {
    printf("PID   PPID  CMD\n");
    print_tree(1, 0, 0);  // Start from the init process with PID 1
    return 0;
}

void print_tree(int parent_pid, int indent, int depth) {
    if (depth >= MAX_DEPTH) {
        fprintf(stderr, "Max depth exceeded.\n");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "/proc/%d", parent_pid);

    DIR *dir = opendir(path);
    if (dir == NULL) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            int child_pid = atoi(entry->d_name);
            if (child_pid > 0) {
                char cmd_path[256];
                snprintf(cmd_path, sizeof(cmd_path), "/proc/%d/cmdline", child_pid);

                int cmd_file = open(cmd_path, O_RDONLY);
                if (cmd_file != -1) {
                    char cmd[256];
                    int bytes_read = read(cmd_file, cmd, sizeof(cmd) - 1);
                    close(cmd_file);

                    if (bytes_read > 0) {
                        cmd[bytes_read] = '\0';

                        // Replace null characters with spaces in the command
                        for (int i = 0; i < bytes_read; i++) {
                            if (cmd[i] == '\0') {
                                cmd[i] = ' ';
                            }
                        }

                        // Print process information with indentation
                        printf("%-5d%-6d%-*s%s\n", child_pid, parent_pid, indent, "", cmd);
                        print_tree(child_pid, indent + 2, depth + 1);
                    }
                }
            }
        }
    }

    closedir(dir);
}
