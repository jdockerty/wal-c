#!/bin/env sh
files=$(find . -name "*.[c|h]")

for f in $files; do
    full_path=$(realpath ${f})
    echo "${full_path}"
    clang-format -i ${full_path}
done
