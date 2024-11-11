#include "wal.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool show_deletes;

#define DELETE_OP "DELETE";
#define INSERT_OP "INSERT";

#define BUF_SIZE 65536

char *operation_to_string(int op) {
  if (op == DELETE) {
    return DELETE_OP;
  } else {
    return INSERT_OP;
  }
}

int count_entries(char *input) {
  int count = 0;
  char *kv_pair = strtok(input, ",");

  while (kv_pair != NULL) {
    count++;
    kv_pair = strtok(NULL, ",");
  }

  return count;
}

void parse_input(SegmentEntry *entries, char *input) {

  int index = 0;
  char *kv_pair = strtok(input, ",");

  while (kv_pair != NULL) {
    char *pair = strchr(kv_pair, '=');
    if (pair != NULL) {
      *pair = '\0';                            // Split the string at '='
      entries[index].key = strdup(kv_pair);    // Duplicate key
      entries[index].value = strdup(pair + 1); // Duplicate value
      index++;
    }
    kv_pair = strtok(NULL, ",");
  }
}

// Fast line counter for knowing the number of entries within a WAL file
// upfront.
//
// The WAL files are never "closed" and encoded with the contained number of
// entries at the moment, so there are no magical bytes to read for metadata to
// know how many entries are contained ahead of time.
//
// Fast impl ref:
// https://stackoverflow.com/questions/12733105/c-function-that-counts-lines-in-file
int count_lines(FILE *file) {
  char buf[BUF_SIZE];
  int counter = 0;
  for (;;) {
    size_t res = fread(buf, 1, BUF_SIZE, file);
    if (ferror(file))
      return -1;

    int i;
    for (i = 0; i < res; i++)
      if (buf[i] == '\n')
        counter++;

    if (feof(file))
      break;
  }

  return counter;
}

int handle_op(FILE *wal_file, SegmentEntry entries[], int count, int op) {
  int i = 0;
  int bytes = 0;
  for (i = 0; i < count; i++) {
    int key_len = strlen(entries[i].key);
    int value_len = strlen(entries[i].value);
    bytes += fwrite(&op, sizeof(int), 1, wal_file);
    bytes += fwrite(&key_len, sizeof(int), 1, wal_file);
    bytes += fwrite(&value_len, sizeof(int), 1, wal_file);
    bytes += fwrite(entries[i].key, sizeof(char), key_len, wal_file);
    bytes += fwrite(entries[i].value, sizeof(char), value_len, wal_file);
    bytes += fwrite("\n", sizeof(char), 1, wal_file);
  }
  return bytes;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    fprintf(stderr, "No command given\n");
    exit(EXIT_FAILURE);
  }

  int opt;

  while ((opt = getopt(argc, argv, "s")) != -1) {
    switch (opt) {
    case 's':
      fprintf(stderr, "Deletes will be shown\n");
      show_deletes = true;
      break;
    case '?':
      fprintf(stderr, "Unknown option\n");
      exit(EXIT_FAILURE);
    default:
      fprintf(stderr, "Usage here\n");
      exit(EXIT_FAILURE);
    }
  }

  // Handle subcommands being passed
  if (optind < argc) {
    char *subcmd = argv[optind];
    if (strcmp(subcmd, "write") == 0) {
      char *wal_file_path = argv[optind + 1];
      if (wal_file_path == NULL) {
        fprintf(stderr, "Must provide a path to a WAL file");
        exit(EXIT_FAILURE);
      }
      fprintf(stderr, "Opened %s\n", wal_file_path);

      char *input = argv[optind + 2];
      if (input == NULL) {
        fprintf(stderr, "key-value input is required.\n");
        exit(EXIT_FAILURE);
      }

      // Open the file for appending at the end.
      FILE *wal_file = fopen(wal_file_path, "a+");
      if (wal_file == NULL) {
        fprintf(stderr, "Unable to open %s\n", wal_file_path);
        exit(EXIT_FAILURE);
      }

      if (is_closed(wal_file)) {
        fprintf(stderr, "Cannot write to closed file.\n");
        exit(EXIT_FAILURE);
      }

      // Counting the entries will modify the string, make a copy of it
      // that we can use later.
      char input_to_parse[strlen(input)];
      strcpy(input_to_parse, input);
      // Add a null character to the end to delimit the end of the string.
      input_to_parse[strlen(input)] = '\0';

      // TODO: do not modify the string during entry count so that we can
      // avoid copying it above.
      int count = count_entries(input);

      SegmentEntry entries[count];
      parse_input(entries, input_to_parse);

      // If the file doesn't have the header, it should be written. This is
      // the first time the file has been written to.
      if (!has_header(wal_file)) {
        write_header(wal_file);
      }

      enum Operation operation_type = INSERT;
      int bytes = handle_op(wal_file, entries, count, operation_type);
      fclose(wal_file);
      fprintf(stderr, "Wrote %d bytes\n", bytes);
    } else if (strcmp(subcmd, "delete") == 0) {
      char *wal_file_path = argv[optind + 1];
      if (wal_file_path == NULL) {
        fprintf(stderr, "Must provide a path to a WAL file");
        exit(EXIT_FAILURE);
      }
      fprintf(stderr, "Opened %s\n", wal_file_path);

      char *input = argv[optind + 2];
      if (input == NULL) {
        fprintf(stderr, "key-value input is required.\n");
        exit(EXIT_FAILURE);
      }

      // Open the file for appending at the end.
      FILE *wal_file = fopen(wal_file_path, "a+");
      if (wal_file == NULL) {
        fprintf(stderr, "Unable to open %s\n", wal_file_path);
        exit(EXIT_FAILURE);
      }

      if (is_closed(wal_file)) {
        fprintf(stderr, "Cannot write to closed file.\n");
        exit(EXIT_FAILURE);
      }

      // Counting the entries will modify the string, make a copy of it
      // that we can use later.
      char input_to_parse[strlen(input)];
      strcpy(input_to_parse, input);
      // Add a null character to the end to delimit the end of the string.
      input_to_parse[strlen(input)] = '\0';

      // TODO: do not modify the string during entry count so that we can
      // avoid copying it above.
      int count = count_entries(input);

      SegmentEntry entries[count];
      parse_input(entries, input_to_parse);

      // If the file doesn't have the header, it should be written. This is
      // the first time the file has been written to.
      if (!has_header(wal_file)) {
        write_header(wal_file);
      }

      enum Operation operation_type = DELETE;
      int bytes = handle_op(wal_file, entries, count, operation_type);
      fclose(wal_file);
      fprintf(stderr, "Wrote %d bytes\n", bytes);

    } else if (strcmp(subcmd, "replay") == 0) {
      char *wal_path = argv[optind + 1];
      if (wal_path == NULL) {
        fprintf(stderr, "Must provide a directory.\n");
        exit(EXIT_FAILURE);
      }

      FILE *wal_file;
      wal_file = fopen(wal_path, "r");
      if (wal_file == NULL) {
        exit(EXIT_FAILURE);
      }

      if (!has_header(wal_file)) {
        fprintf(stderr,
                "%s is not a WAL file, the expected header was not present.\n",
                wal_path);
        exit(EXIT_FAILURE);
      }

      fprintf(stderr, "Attempting to replay %s\n", wal_path);

      int num_entries;
      if (is_closed(wal_file)) {
        seek_after_header(wal_file);
        // Read the number of entries from the metadata footer.
        num_entries = entries_metadata(wal_file);
      } else {
        seek_after_header(wal_file);
        // When the file has not been closed, there is no metadata to read
        // so we must resort to counting the number of entries ourselves.
        num_entries = count_lines(wal_file);
      }

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
      for (i = 0; i < num_entries; i++) {
        int key_len, value_len;

        enum Operation operation_type;
        fread(&operation_type, sizeof(int), 1, wal_file);

        // By default, deletes should be skipped.
        if (operation_type == DELETE && !show_deletes) {
          continue;
        }

        // Read the key/value lengths that were encoded.
        fread(&key_len, sizeof(int), 1, wal_file);
        fread(&value_len, sizeof(int), 1, wal_file);

        char *key = malloc(key_len);
        char *value = malloc(value_len);

        fread(key, sizeof(char), key_len, wal_file);
        fread(value, sizeof(char), value_len, wal_file);
        printf("[%s] %s=%s\n", operation_to_string(operation_type), key, value);
        free(key);
        free(value);

        // Move the cursor by 1 character for the newline, the result can
        // simply be discarded.
        fgetc(wal_file);
      }
      fclose(wal_file);
    } else if (strcmp(subcmd, "close") == 0) {
      char *wal_path = argv[optind + 1];
      if (wal_path == NULL) {
        fprintf(stderr, "Must provide a directory.\n");
        exit(EXIT_FAILURE);
      }

      FILE *wal_file;
      wal_file = fopen(wal_path, "a+");
      if (wal_file == NULL) {
        exit(EXIT_FAILURE);
      }

      fseek(wal_file, 0, SEEK_SET);

      if (!has_header(wal_file)) {
        fprintf(stderr,
                "%s is not a WAL file, the expected header was not present.\n",
                wal_path);
        exit(EXIT_FAILURE);
      }

      int entries = count_lines(wal_file);
      // Seek to the end of the file so that we can encode the metadata.
      fseek(wal_file, 0, SEEK_END);

      write_metadata(wal_file, entries);
      fclose(wal_file);
      fprintf(stderr, "%s has been closed and marked as immutable.\n",
              wal_path);
    } else {
      printf("Unrecognised command: %s\n", subcmd);
      exit(EXIT_FAILURE);
    }
  } else {
    fprintf(stderr, "No subcommand provided.\n");
    fprintf(stderr, "Expected either: 'write', 'delete', 'replay', or 'close'\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}
