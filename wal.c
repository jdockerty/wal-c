#include "wal.h"
#include "stdio.h"
#include <stdio.h>
#include <string.h>

bool has_header(FILE *wal_file) {
    char header[strlen(WAL_HEADER) + 1]; // header + NULL terminator
    fseek(wal_file, 0, SEEK_SET);
    fread(header, sizeof(char), strlen(WAL_HEADER), wal_file);
    header[2] = '\0';
    return strcmp(header, WAL_HEADER) == 0;
}

void write_header(FILE *wal_file) {
    fwrite(WAL_HEADER, sizeof(WAL_HEADER[0]), strlen(WAL_HEADER), wal_file);
}
