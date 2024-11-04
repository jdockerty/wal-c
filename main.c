#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("No command given\n");
        exit(1);
    }

    char* subcmd = argv[1];
    if (strcmp(subcmd, "create") == 0) {
        char* dir = argv[2];
        char* wal_path;
        asprintf(&wal_path, "%s/0.wal", dir);

        FILE* file = fopen(wal_path, "w+");
        if (file != NULL) {
            printf("Created %s\n", wal_path);
        } else {
            if (errno == 14) {
                fprintf(stderr, "No path provided\n");
            } else {
                fprintf(stderr, "Unable to create %s, %d\n", wal_path, errno);
            }
            exit(1);
        }
    } else if (strcmp(subcmd, "write") == 0) {
        printf("write!");
    } else if (strcmp(subcmd, "replay") == 0) {
        printf("replay!");
    } else {
        printf("Unrecognised command: %s\n", subcmd);
        exit(1);
    }

    return 0;
}
