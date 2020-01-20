/*
 * Red Archive
 * MIT License
 * Copyright (c) 2020 Jacob Gelling
 */

#include "cli.h"

int main(const int argc, char *argv[]) {
    // Set return code to success
    int return_code = EXIT_SUCCESS;

    switch (argc) {
        // No arguments provided
        case 1:
            printf("Red Archive %d.%d\n", REDARCHIVE_VERSION_MAJOR, REDARCHIVE_VERSION_MINOR);
            printf("MIT License\n");
            printf("Copyright (c) 2020 Jacob Gelling\n\n");
            printf("  To unpack an archive into a folder:\n");
            printf("  %s -u archive folder\n\n", argv[0]);
            printf("  To pack a folder into an archive:\n");
            printf("  %s -p folder archive\n", argv[0]);
            break;

        // Correct number of arguments provided
        case 4:
            if (strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--unpack") == 0) {
                return_code = unpack(argv[2], argv[3]);
            } else if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--pack") == 0) {
                return_code = pack(argv[2], argv[3]);
            } else {
                fprintf(stderr, "Unknown option %s\n", argv[1]);
                return EXIT_FAILURE;
            }
            break;

        // Incorrect number of arguments provided
        default:
            fprintf(stderr, "Incorrect number of arguments\n");
            return EXIT_FAILURE;
    }

    return return_code;
}
