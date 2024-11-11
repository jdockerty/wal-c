# wal-c

A Write-Ahead Log (WAL) written in C.

## Concept

The WAL has an extremely simple design, it should be able to write arbitrary 
key-value pairs and replay the inserted values back.

## Usage

A CLI tool is provided to act as an entrypoint to the WAL operations[^1].

[^1]: A WAL works much better for longer running operations than for a CLI, so
this is really a glorified file read/writer.

- `wal-c write PATH_TO_FILE k=v,...`: Write an arbitrary number of key-value pairs to the file as an insertion operation.
- `wal-c delete PATH_TO_FILE k=v,...`: Write an arbitrary number of key-value pairs to the file as a delete operation.
- `wal-c [-s] replay PATH_TO_FILE`: Replay the values written into the WAL directory. When `-s` is given, deletes will be shown.
- `wal-c close PATH_TO_FILE`: Closes a file, encoding metadata and marking it as immutable. 
