# wal-c

A Write-Ahead Log (WAL) written in C.

## Concept

The WAL has an extremely simple design, it should be able to write arbitrary 
key-value pairs as either an insertion or deletion operation.

## Usage

A CLI tool is provided to act as an entrypoint to the WAL operations.

- `wal-c create <path/to/dir>`: Create a WAL file within the given directory
- `wal-c write <path/to/dir> k=v,k=v,...`: Write an arbitrary number of key-value pairs to the WAL directory
- `wal-c replay <path>`: Replay the values written into the WAL directory
