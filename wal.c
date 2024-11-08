#include "wal.h"
#include "stdio.h"
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

void seek_after_header(FILE *wal_file) {
  fseek(wal_file, strlen(WAL_HEADER), SEEK_SET);
}

void write_metadata(FILE *wal_file, int num_entries) {
  fwrite(WAL_FOOTER, strlen(WAL_FOOTER), 1, wal_file);
  fwrite(&num_entries, sizeof(int), 1, wal_file);
}

void seek_to_metadata(FILE *wal_file) {
  int footer_start = strlen(WAL_FOOTER) + sizeof(int);
  fseek(wal_file, -footer_start, SEEK_END);
}

bool is_closed(FILE *wal_file) {
  seek_to_metadata(wal_file);
  char footer[strlen(WAL_FOOTER) + 1]; // footer + NULL terminator
  fread(footer, sizeof(char), strlen(WAL_FOOTER), wal_file);
  footer[2] = '\0';
  return strcmp(footer, WAL_FOOTER) == 0;
}

int entries_metadata(FILE *wal_file) {
  seek_to_metadata(wal_file);

  // Jump the cursor over the footer which is known to exist at this point.
  fseek(wal_file, strlen(WAL_FOOTER), SEEK_CUR);

  int num_entries;
  fread(&num_entries, sizeof(int), 1, wal_file);
  return num_entries;
}
