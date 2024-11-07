#ifndef WAL_H_
#define WAL_H_

typedef struct S {
    char* key;
    char* value;
} SegmentEntry;

typedef struct W {} Wal;

// Creates a WAL path with an input directory and ID.
char* generate_wal_path(char* dir, int id);

Wal new_wal(char* path);

#endif
