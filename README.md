# wal-c

A Write-Ahead Log (WAL) written in C.

## Concept

The WAL has an extremely simple design, it should be able to write arbitrary 
key-value pairs as either an insertion or deletion operation.

## Usage

A CLI tool is provided to act as an entrypoint to the WAL operations[^1].

[^1]: A WAL works much better for longer running operations than for a CLI, so
this is really a glorified file read/writer.

- `wal-c write PATH_TO_FILE k=v,k=v,...`: Write an arbitrary number of key-value pairs to the file 
- `wal-c replay PATH_TO_FILE`: Replay the values written into the WAL directory
- `wal-c close PATH_TO_FILE`: Closes a file, encoding metadata and marking it as immutable. 
