files = $(find . -name "*.[c|h]")

build:
	gcc -O -g main.c wal.c -o wal-c

fmt:
	@scripts/fmt.sh
