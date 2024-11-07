#ifndef WAL_H_
#define WAL_H_

#include <stdio.h>
#include <stdbool.h>

#define WAL_HEADER "w0"

typedef struct S {
    char* key;
    char* value;
} SegmentEntry;


// Check whether a WAL file has the expected header.
bool has_header(FILE* wal_file);

// Write the WAL_HEADER to a WAL file.
void write_header(FILE* wal_file);

#endif
