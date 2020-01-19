/*
 * Red Archive
 * MIT License
 * Copyright (c) 2020 Jacob Gelling
 */

#include "cli.h"

int main(int argc, char *argv[]) {
    int return_code = 0;

    // Handle argumants
    if (argc == 1) {
        printf("Red Archive %d.%d\n", REDARCHIVE_VERSION_MAJOR, REDARCHIVE_VERSION_MINOR);
        printf("MIT License\n");
        printf("Copyright (c) 2020 Jacob Gelling\n\n");
        printf("  To unpack an archive into a folder:\n");
        printf("  %s -u [archive] [folder]\n\n", argv[0]);
        printf("  To pack a folder into an archive:\n");
        printf("  %s -p [folder] [archive]\n", argv[0]);
    } else if (argc == 4) {
        if (strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--unpack") == 0) {
            return_code = unpack(argv[2], argv[3]);
        } else if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--pack") == 0) {
            return_code = pack(argv[2], argv[3]);
        } else {
            return_code = 1;
            printf("Unknown option %s\n", argv[1]);
        }
    } else {
        return_code = 1;
        printf("Unknown number of arguments\n");
    }

    // Display return code if other than 0
    if(return_code) {
        printf("Error code %i occourred\n", return_code);
    }
    return return_code;
}
