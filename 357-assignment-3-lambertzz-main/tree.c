#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

void print_dir(const char *dir_name, int hidden, int level, int size);
int compare(const void *a, const void *b);
void format(const char *name,struct stat *st, int size, int level,   int last);
int num_dir = 0;
int num_files = 0;


int compare(const void *a, const void *b) {
    char *file1 = *((char**) a);
    char *file2 = *((char**) b);
    return strcmp(file1, file2);
}

void format(const char *name, struct stat *st, int size, int level,  int last) {

    int i = 0;

    for (i; i < level; i++) {
        printf("|   ");
    }

    if (last != 0) {
        if (size) {

            printf("`-- [%10lld]  %s\n", (long)st->st_size, name);

        } else {

            printf("`-- %s\n", name);
        }

    } else {
        if (size != 0) {

            printf("|-- [%10lld]  %s\n", (long)st->st_size, name);

        } else {

            printf("|-- %s\n", name);

        }
    }
}

void print_dir(const char *dir_name, int hidden, int level, int size) {

    DIR *dir = opendir(dir_name);

    if (dir == NULL) {
        exit(1);
    }

    
    if (chdir(dir_name) != 0) {
        exit(1);
    }

    struct dirent *file = readdir(dir);;
    struct stat *stats = (struct stat *)calloc(300, sizeof(struct stat));
    char **array = (char **)calloc(300, sizeof(char *));
    int n = 0;
    *array = file->d_name;
   
    while ((file = readdir(dir)) != NULL) {

        if (!hidden && file->d_name[0] == '.') {
            continue;
        }

        if (strcmp(file->d_name, "..") == 0 || strcmp(file->d_name, ".") == 0) {
            continue;
        }

        array[n] = strdup(file->d_name);

        if (stat(file->d_name, &stats[n]) != 0) {
            free(array[n]);
            exit(1);
        }

        n++;
        
    }

    closedir(dir);

  
    qsort(array, n, sizeof(char *), compare);


    int i = 0;
    for (i; i < n; i++) {

        int last = (i == n - 1);
        format(array[i], &stats[i], size, level,  last);


        if (S_ISDIR(stats[i].st_mode)) {
            num_dir++;
            
            print_dir(array[i], hidden,level + 1, size );

            if (chdir("..") != 0) {
                exit(1);
            }

        } else {
                num_files++;
            }
        free(array[i]);
            } 
        free(stats);
        free(array);
}

int main(int argc, char *argv[]) {
    int hidden;
    char *dir_name;
    int size;


    int i = 1;

    for (i; i < argc; i++) {

        if (strcmp(argv[i], "-s") == 0) {
            size = 1;

        } else if (strcmp(argv[i], "-a") == 0) {
            hidden = 1;

        } else {
            dir_name = argv[i];
        }
    }

    printf("%s\n", dir_name);
    print_dir(dir_name, hidden, 0, size);
    printf("%d directories, %d files\n", num_dir, num_files);
    return 0;
}