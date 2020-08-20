/*
 * Red Archive
 * MIT License
 * Copyright (c) 2020 Jacob Gelling
 */

#include "cli.h"

int main(const int argc, char *argv[]) {
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
                if (unpack(argv[2], argv[3]) != 1) {
                    return EXIT_FAILURE;
                }
            } else if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--pack") == 0) {
                if (pack(argv[2], argv[3]) != 1) {
                    return EXIT_FAILURE;
                }
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

    return EXIT_SUCCESS;
}
