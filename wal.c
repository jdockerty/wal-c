#include "wal.h"
#include "stdio.h"

char* generate_wal_path(char *dir, int id) {
    char* path;
    asprintf(&path, "%s/%d.wal", dir, id);
    return path;
}
