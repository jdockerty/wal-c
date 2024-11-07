#include "wal.h"
#include "stdio.h"
#include <stdio.h>
#include <string.h>

bool has_header(FILE *wal_file) {
    fseek(wal_file, SEEK_SET, 0);
    char header[2];
    fread(header, sizeof(char), 2, wal_file);

    return strcmp(header, WAL_HEADER) == 0;
}

void write_header(FILE *wal_file) {
    fwrite(WAL_HEADER, sizeof(char), 2, wal_file);
}
