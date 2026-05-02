#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int shouldSkip(char *ext, char excluded[][50], int count)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(ext, excluded[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void removeNewline(char *str)
{
    int len = strlen(str);

    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: ./child1 directory exclude_file\n");
        return 1;
    }

    char *folderPath = argv[1];
    char *excludeFile = argv[2];

    char excluded[100][50];
    int excludeCount = 0;

    FILE *fp = fopen(excludeFile, "r");

    if (fp != NULL) {
        while (fgets(excluded[excludeCount], 50, fp) != NULL) {
            removeNewline(excluded[excludeCount]);

            if (excluded[excludeCount][0] == '.') {
                memmove(excluded[excludeCount],
                        excluded[excludeCount] + 1,
                        strlen(excluded[excludeCount]));
            }

            excludeCount++;
        }
        fclose(fp);
    }

    DIR *dir = opendir(folderPath);

    if (dir == NULL) {
        perror("Directory open failed");
        return 1;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char *dot = strrchr(entry->d_name, '.');

        if (dot == NULL) {
            continue;
        }

        char ext[50];
        strcpy(ext, dot + 1);

        if (shouldSkip(ext, excluded, excludeCount)) {
            printf("Child 1 skipped: %s\n", entry->d_name);
            continue;
        }

        char newFolder[300];
        snprintf(newFolder, sizeof(newFolder), "%s/%s", folderPath, ext);
        mkdir(newFolder, 0777);

        char oldPath[300];
        char newPath[300];

        snprintf(oldPath, sizeof(oldPath), "%s/%s", folderPath, entry->d_name);
        snprintf(newPath, sizeof(newPath), "%s/%s/%s", folderPath, ext, entry->d_name);

        if (rename(oldPath, newPath) == 0) {
            printf("Child 1 moved %s to %s folder\n", entry->d_name, ext);
        }
    }

    closedir(dir);

    printf("Child 1 finished file categorization.\n");

    return 0;
}
