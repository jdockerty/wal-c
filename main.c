#include "wal.h"
#include <_string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//char* parse_key(char* input) {}
//
//char* parse_value(char* input) {}

void parse_input(SegmentEntry* entries, char* input) {
    char* buf;
    SegmentEntry entry;
    char* token;

    // Iterate over the input string until there are no more delimited tokens
    // remaining.
    while (( (token = strsep(&input, ",")) != NULL)) {
        int i;
        // The values are delimited via '=', so we must iterate over the returned
        // token twice.
        for (i=0; i<2; i++) {
            char* kv = strsep(&token, "=");
            printf("%s\n", kv);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "No command given\n");
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
            fprintf(stderr, "Created %s\n", wal_path);
        } else {
            fprintf(stderr, "Unable to create WAL file.\n");
            exit(1);
        }
    } else if (strcmp(subcmd, "write") == 0) {
        char* wal_file_path = argv[2];
        if (wal_file_path == NULL) {
            fprintf(stderr, "Must provide a path to a WAL file");
            exit(1);
        }
        fprintf(stderr, "Opened %s\n", wal_file_path);

        char* input = argv[3];
        if (input == NULL) {
            fprintf(stderr, "key-value input is required.\n");
            exit(1);
        }
        SegmentEntry* entries;
        parse_input(entries, input);

        // Open the file for appending at the end.
        FILE* wal_file = fopen(wal_file_path, "a");
        if (wal_file == NULL) {
            fprintf(stderr, "Unable to open %s\n", wal_file_path);
            exit(1);
        }
    } else if (strcmp(subcmd, "replay") == 0) {
        printf("replay!");
    } else {
        printf("Unrecognised command: %s\n", subcmd);
        exit(1);
    }

    return 0;
}

