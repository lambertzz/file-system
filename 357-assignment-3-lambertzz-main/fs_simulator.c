#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>

typedef struct {
    uint32_t inode;
    char type;
} Inode;

typedef struct {
    uint32_t inode;
    char name[33];
} DirectoryEntry;

Inode inodes[1024];
char fs_dir[256];
int curr_i = 0;

// Utility: Convert uint32_t to string
char *uint32_to_str(uint32_t i) {
    int length = snprintf(NULL, 0, "%u", i);
    char* str = malloc(length + 1);
    snprintf(str, length + 1, "%u", i);
    return str;
}

// Change directory check
int dir_check(char *path) {
    return chdir(path);
}

// Load inode list from 'inodes_list' file
void i_list() {
    FILE *fp = fopen("inodes_list", "rb");
    if (!fp) {
        printf("Can't open list\n");
        exit(1);
    }

    for (int i = 0; i < 1024; i++) {
        if (fread(&inodes[i].inode, sizeof(uint32_t), 1, fp) != 1) break;
        if (fread(&inodes[i].type, 1, 1, fp) != 1) break;
    }
    fclose(fp);
}

// Save inode list to 'inodes_list' file
void save_i() {
    FILE *fp = fopen("inodes_list", "wb");
    if (!fp) {
        printf("Can't open list\n");
        exit(1);
    }

    for (int i = 0; i < 1024; i++) {
        if (inodes[i].type == 0) break;
        fwrite(&inodes[i].inode, sizeof(uint32_t), 1, fp);
        fwrite(&inodes[i].type, 1, 1, fp);
    }
    fclose(fp);
}

// List directory contents
void list_dir(uint32_t inode) {
    char *file = uint32_to_str(inode);
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        printf("Can't open directory\n");
        free(file);
        return;
    }

    DirectoryEntry entry;
    while (fread(&entry.inode, sizeof(uint32_t), 1, fp) == 1) {
        fread(entry.name, 1, 32, fp);
        entry.name[32] = '\0';
        printf("%u %s\n", entry.inode, entry.name);
    }
    fclose(fp);
    free(file);
}

// Switch to a different directory
void switch_dir(char *name) {
    char *file = uint32_to_str(curr_i);
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        printf("Unable to open dir\n");
        free(file);
        return;
    }

    DirectoryEntry entry;
    while (fread(&entry.inode, sizeof(uint32_t), 1, fp)) {
        fread(entry.name, 1, 32, fp);
        entry.name[32] = '\0';
        if (strcmp(entry.name, name) == 0) {
            if (inodes[entry.inode].type == 'd') {
                curr_i = entry.inode;
            } else {
                printf("Not a directory\n");
            }
            fclose(fp);
            free(file);
            return;
        }
    }
    printf("Directory %s not found\n", name);
    fclose(fp);
    free(file);
}

// Create a new file
void create_file(char *name) {
    for (int i = 0; i < 1024; i++) {
        if (inodes[i].type == 0) {
            inodes[i].inode = i;
            inodes[i].type = 'f';

            char *file = uint32_to_str(i);
            FILE *fp = fopen(file, "wb");
            if (!fp) {
                printf("Unable to create file\n");
                free(file);
                return;
            }
            fprintf(fp, "%s\n", name);
            fclose(fp);
            free(file);
            return;
        }
    }
    printf("No available inodes\n");
}

// Create a new directory
void create_dir(char *name) {
    for (int i = 0; i < 1024; i++) {
        if (inodes[i].type == 0) {
            inodes[i].inode = i;
            inodes[i].type = 'd';

            char *file = uint32_to_str(i);
            FILE *fp = fopen(file, "wb");
            if (!fp) {
                printf("Unable to create directory\n");
                free(file);
                return;
            }
            DirectoryEntry entry = {i, "."};
            fwrite(&entry, sizeof(DirectoryEntry), 1, fp);
            entry.inode = curr_i;
            strcpy(entry.name, "..");
            fwrite(&entry, sizeof(DirectoryEntry), 1, fp);
            fclose(fp);
            free(file);
            return;
        }
    }
    printf("No available inodes\n");
}

// Main prompt loop
void prompt() {
    char command[256];
    while (1) {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) == NULL) break;
        char *cmd = strtok(command, " \n");

        if (strcmp(cmd, "exit") == 0) {
            save_i();
            break;
        } else if (strcmp(cmd, "cd") == 0) {
            char *name = strtok(NULL, " \n");
            if (name) switch_dir(name);
            else printf("Error: No directory specified\n");
        } else if (strcmp(cmd, "ls") == 0) {
            list_dir(curr_i);
        } else if (strcmp(cmd, "mkdir") == 0) {
            char *name = strtok(NULL, " \n");
            if (name) create_dir(name);
            else printf("Error: No name specified\n");
        } else if (strcmp(cmd, "touch") == 0) {
            char *name = strtok(NULL, " \n");
            if (name) create_file(name);
            else printf("Error: No name specified\n");
        } else {
            printf("Unknown command\n");
        }
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filesystem_directory>\n", argv[0]);
        return 1;
    }
    strncpy(fs_dir, argv[1], sizeof(fs_dir) - 1);
    if (dir_check(fs_dir) != 0) {
        printf("Failed to change directory\n");
        return 1;
    }
    i_list();
    prompt();
    return 0;
}
