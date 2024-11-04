#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("No command given\n");
        exit(1);
    }

    char* subcmd = argv[1];
    if (strcmp(subcmd, "create") == 0) {
        printf("create!");
    } else if (strcmp(subcmd, "write") == 0) {
        printf("write!");
    } else if (strcmp(subcmd, "replay") == 0) {
        printf("replay!");
    } else {
        printf("Unrecognised command: %s\n", subcmd);
        exit(1);
    }

    return 0;
}
