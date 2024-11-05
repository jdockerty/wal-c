#include "wal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//char* parse_key(char* input) {}
//
//char* parse_value(char* input) {}

void parse_input(SegmentEntry* entries, char* input) {
    char* kv_pair = strtok(input, ",");
    int index = 0;

    while (kv_pair != NULL) {

        char* pair = strchr(kv_pair, '=');

        if (pair != NULL) {
            // Separate key and value
            *pair = '\0'; // Split the string at '='
            entries[index].key = strdup(kv_pair); // Duplicate key
            entries[index].value = strdup(pair + 1); // Duplicate value
            index++;
        }
        kv_pair = strtok(NULL, ",");
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "No command given\n");
        exit(1);
    }

    char* subcmd = argv[1];
    if (strcmp(subcmd, "write") == 0) {
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
        SegmentEntry entries[5];
        parse_input(entries, input);

        // Open the file for appending at the end.
        FILE* wal_file = fopen(wal_file_path, "a+");
        if (wal_file == NULL) {
            fprintf(stderr, "Unable to open %s\n", wal_file_path);
            exit(1);
        }

        int j = 0;
        for (j=0; j < 2; j++) {
            int key_len = strlen(entries[j].key);
            int value_len = strlen(entries[j].value);
            fwrite(&key_len, sizeof(int), 1, wal_file);
            fwrite(entries[j].key, sizeof(char), key_len, wal_file);
            fwrite(&value_len, sizeof(int), 1, wal_file);
            fwrite(entries[j].value, sizeof(char), value_len, wal_file);
            fwrite("\n", sizeof(char), 1, wal_file);
        }
        fclose(wal_file);
    } else if (strcmp(subcmd, "replay") == 0) {
        // TODO
        char* dir = argv[2];
        if (dir == NULL) {
            fprintf(stderr, "Must provide a directory.\n");
            exit(1);
        }
        char* wal_path = generate_wal_path(dir, 0);

        FILE * wal_file;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;

        wal_file = fopen(wal_path, "r");
        if (wal_file == NULL)
            exit(EXIT_FAILURE);

        while ((read = getline(&line, &len, wal_file)) != -1) {
            printf("Retrieved line of length %zu:\n", read);
            SegmentEntry entry;
            printf("%s", line);
        }

        fclose(wal_file);
    } else {
        printf("Unrecognised command: %s\n", subcmd);
        exit(1);
    }

    return 0;
}

