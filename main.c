#include "wal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("No command given\n");
        exit(1);
    }

    char* subcmd = argv[1];
    if (strcmp(subcmd, "create") == 0) {
        char* dir = argv[2];
        if (dir == NULL) {
            fprintf(stderr, "Must provide a directory.\n");
            exit(1);
        }

        char* wal_path = generate_wal_path(dir, 0);

        FILE* file = fopen(wal_path, "w+");
        if (file != NULL) {
            printf("Created %s\n", wal_path);
        } else {
            fprintf(stderr, "Unable to create WAL file.\n");
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
