#ifndef WAL_H_
#define WAL_H_

#include <stdio.h>
#include <stdbool.h>

// Magic bytes that are expected at the start of a WAL file.
#define WAL_HEADER "w1"

typedef struct S {
    char* key;
    char* value;
} SegmentEntry;


// Check whether a WAL file has the expected header.
bool has_header(FILE* wal_file);

// Write the WAL_HEADER to a WAL file.
void write_header(FILE* wal_file);

#endif
