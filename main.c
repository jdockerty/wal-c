#include "wal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 65536

int parse_input(SegmentEntry* entries, char* input) {
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

    return index;
}


// Fast line counter for knowing the number of entries within a WAL file upfront.
//
// The WAL files are never "closed" and encoded with the contained number of
// entries at the moment, so there are no magical bytes to read for metadata to
// know how many entries are contained ahead of time.
//
// Fast impl ref: https://stackoverflow.com/questions/12733105/c-function-that-counts-lines-in-file
int count_lines(FILE* file) {
    char buf[BUF_SIZE];
    int counter = 0;
    for(;;)
    {
        size_t res = fread(buf, 1, BUF_SIZE, file);
        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }

    return counter;
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
        SegmentEntry entries[10]; // TODO: entry count ahead of time?
        int count = parse_input(entries, input);

        // Open the file for appending at the end.
        FILE* wal_file = fopen(wal_file_path, "a+");
        if (wal_file == NULL) {
            fprintf(stderr, "Unable to open %s\n", wal_file_path);
            exit(1);
        }

        int i = 0;
        for (i=0; i < count; i++) {
            int key_len = strlen(entries[i].key);
            printf("%s is %d\n", entries[i].key, key_len);
            int value_len = strlen(entries[i].value);
            printf("%s is %d\n", entries[i].value, value_len);
            fwrite(&key_len, sizeof(int), 1, wal_file);
            fwrite(&value_len, sizeof(int), 1, wal_file);
            fwrite(entries[i].key, sizeof(char), key_len, wal_file);
            fwrite(entries[i].value, sizeof(char), value_len, wal_file);
            fwrite("\n", sizeof(char), 1, wal_file);
        }
        fclose(wal_file);
    } else if (strcmp(subcmd, "replay") == 0) {
        char* dir = argv[2];
        if (dir == NULL) {
            fprintf(stderr, "Must provide a directory.\n");
            exit(1);
        }

        char* wal_path = generate_wal_path(dir, 0);

        FILE * wal_file;
        wal_file = fopen(wal_path, "r");
        if (wal_file == NULL)
            exit(EXIT_FAILURE);

        int line_count = count_lines(wal_file);
        // The cursor must be reset after counting the lines, as this takes
        // us to EOF.
        fseek(wal_file, SEEK_SET, 0);

        int i;
        for (i=0; i < line_count; i++) {
            int key_len, value_len;

            // Read the key/value lengths that were encoded.
            fread(&key_len, sizeof(int), 1, wal_file);
            fread(&value_len, sizeof(int), 1, wal_file);

            char* key = malloc(key_len);
            char* value = malloc(value_len);

            fread(key, sizeof(char), key_len, wal_file);
            fread(value, sizeof(char), value_len, wal_file);
            printf("%s=%s\n", key, value);
            free(key);
            free(value);

            // Move the cursor by 1 character for the newline, the result can
            // simply be discarded.
            fgetc(wal_file);
        }

        fclose(wal_file);
    } else {
        printf("Unrecognised command: %s\n", subcmd);
        exit(1);
    }

    return 0;
}

