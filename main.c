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
        exit(EXIT_FAILURE);
    }

    char* subcmd = argv[1];
    if (strcmp(subcmd, "write") == 0) {
        char* wal_file_path = argv[2];
        if (wal_file_path == NULL) {
            fprintf(stderr, "Must provide a path to a WAL file");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Opened %s\n", wal_file_path);

        char* input = argv[3];
        if (input == NULL) {
            fprintf(stderr, "key-value input is required.\n");
            exit(EXIT_FAILURE);
        }
        SegmentEntry entries[10]; // TODO: entry count ahead of time?
        int count = parse_input(entries, input);

        // Open the file for appending at the end.
        FILE* wal_file = fopen(wal_file_path, "a+");
        if (wal_file == NULL) {
            fprintf(stderr, "Unable to open %s\n", wal_file_path);
            exit(EXIT_FAILURE);
        }

        // If the file doesn't have the header, it should be written. This is
        // the first time the file has been written to.
        if (!has_header(wal_file)) {
            write_header(wal_file);
        }

        int i = 0;
        int bytes = 0;
        for (i=0; i < count; i++) {
            int key_len = strlen(entries[i].key);
            int value_len = strlen(entries[i].value);
            bytes += fwrite(&key_len, sizeof(int), 1, wal_file);
            bytes += fwrite(&value_len, sizeof(int), 1, wal_file);
            bytes += fwrite(entries[i].key, sizeof(char), key_len, wal_file);
            bytes += fwrite(entries[i].value, sizeof(char), value_len, wal_file);
            bytes += fwrite("\n", sizeof(char), 1, wal_file);
        }
        fclose(wal_file);
        fprintf(stderr, "Wrote %d bytes\n", bytes);
    } else if (strcmp(subcmd, "replay") == 0) {
        char* wal_path = argv[2];
        if (wal_path == NULL) {
            fprintf(stderr, "Must provide a directory.\n");
            exit(EXIT_FAILURE);
        }

        FILE * wal_file;
        wal_file = fopen(wal_path, "r");
        if (wal_file == NULL)
            exit(EXIT_FAILURE);

        if (!has_header(wal_file)) {
            fprintf(stderr, "%s is not a WAL file, the expected header was not present.\n", wal_path);
            exit(EXIT_FAILURE);
        }

        int entries_res = fseek(wal_file, -sizeof(int), SEEK_END);
        int num_entries;
        fread(&num_entries, sizeof(int), 1, wal_file);

        // The cursor must be reset after reading the metadata, as this takes us 
        // to near-EOF.
        // We do not reset to the beginning of the file, rather after the header
        // which is known to exist at this point.
        int res = fseek(wal_file, strlen(WAL_HEADER), SEEK_SET);
        if (res != 0) {
            fprintf(stderr, "Unable to seek cursor to after the WAL header.\n");
            exit(EXIT_FAILURE);
        }

        int i;
        for (i=0; i < num_entries; i++) {
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
    } else if (strcmp(subcmd, "close") == 0) {
        char* wal_path = argv[2];
        if (wal_path == NULL) {
            fprintf(stderr, "Must provide a directory.\n");
            exit(EXIT_FAILURE);
        }

        FILE * wal_file;
        wal_file = fopen(wal_path, "a+");
        if (wal_file == NULL) {
            exit(EXIT_FAILURE);
        }

        fseek(wal_file, 0, SEEK_SET);

        if (!has_header(wal_file)) {
            fprintf(stderr, "%s is not a WAL file, the expected header was not present.\n", wal_path);
            exit(EXIT_FAILURE);
        }

        int entries = count_lines(wal_file);
        // Seek to the end of the file so that we can encode the metadata.
        fseek(wal_file, 0, SEEK_END);

        int bytes = fwrite(&entries, sizeof(int), 1, wal_file);
        fclose(wal_file);
        fprintf(stderr, "%s has been closed and marked as immutable.\n", wal_path);
    } else {
        printf("Unrecognised command: %s\n", subcmd);
        exit(EXIT_FAILURE);
    }

    return 0;
}

