#ifndef WAL_H_
#define WAL_H_

#include <stdio.h>
#include <stdbool.h>

// Magic bytes that are expected at the start of a WAL file.
#define WAL_HEADER "w1"

// Magic bytes that are expected at the end of a WAL file.
#define WAL_FOOTER "wf"

typedef struct S {
    char* key;
    char* value;
} SegmentEntry;

// Check whether a WAL file has the expected header.
bool has_header(FILE* wal_file);

// Write the WAL_HEADER to a WAL file.
void write_header(FILE* wal_file);

// Write metadata to the WAL file footer.
void write_metadata(FILE* wal_file, int num_entries);

// Check whether a WAL file has been closed and declared as immutable.
bool is_closed(FILE* wal_file);

// Retrieve the number of entries written to the file. This comes from the
// encoded metadata at the footer of the file.
int entries_metadata(FILE* wal_file);

// Move the file cursor to after the header.
void seek_after_header(FILE* wal_file);

#endif
