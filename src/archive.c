/*
 * Red Archive
 * MIT License
 * Copyright (c) 2020 Jacob Gelling
 */

#include "archive.h"

// Set filename size to length of 8.3 filename with null-terminator
#define FILENAME_SIZE 13

static inline void make_folder(const char *folder_path)  {
    #ifdef _WIN32
        _mkdir(folder_path);
    #else
        mkdir(folder_path, 0777);
    #endif
}

static bool valid_filename_character(const char character) {
    // Check if a valid MS-DOS filename character
    if (
        (character >= 36 && character <= 41)  || // !-)
        (character >= 48 && character <= 57)  || // 0-9
        (character >= 64 && character <= 90)  || // @-Z
        (character >= 94 && character <= 123) || // ^-{
        character == 33  || // !
        character == 45  || // -
        character == 46  || // .
        character == 125 || // }
        character == 126    // ~
    ) {
        return true;
    }
    return false;
}

static char *make_file_path(const char *folder_path, const char *filename) {
    char *file_path = malloc(strlen(filename) + strlen(folder_path) + 2);
    strcpy(file_path, folder_path);
    strcat(file_path, "/");
    strcat(file_path, filename);
    return file_path;
}

static const size_t get_file_size(FILE *file_pointer) {
    fseek(file_pointer, 0, SEEK_END);
    const size_t file_size = ftell(file_pointer);
    fseek(file_pointer, 0, SEEK_SET);
    return file_size;
}

int unpack(const char *archive_path, const char *folder_path) {
    // Open archive
    FILE *archive_pointer = NULL;
    if ((archive_pointer = fopen(archive_path, "rb")) == NULL) {
        fprintf(stderr, "Error opening archive %s\n", archive_path);
        return EXIT_FAILURE;
    }

    // Create folder
    make_folder(folder_path);

    // Unpack all files
    while (1) {
        // Get position in file
        size_t position = ftell(archive_pointer);

        // Read filename
        char filename_buffer[FILENAME_SIZE];
        const size_t filename_read = fread(filename_buffer, 1, FILENAME_SIZE, archive_pointer);

        // Fail if no filename read
        if (filename_read == 0) {
            fclose(archive_pointer);
            fprintf(stderr, "Could not read filename in archive %s\n", archive_path);
            return EXIT_FAILURE;
        }

        // Finish unpacking if end of file byte found
        if (filename_read == 1 && filename_buffer[0] == '\0') {
            break;
        }

        // Ensure filename is valid
        const size_t filename_last_i = filename_read - 1;
        for (int i = 0; i < filename_read; i++) {
            // Break if null-terminator found
            if (i > 0 & filename_buffer[i] == 0) {
                break;
            // Fail if invalid character found or string is unterminated
            } else if (!valid_filename_character(filename_buffer[i]) || (i == filename_last_i && filename_buffer[i] != 0)) {
                fclose(archive_pointer);
                fprintf(stderr, "Invalid filename in archive %s\n", archive_path);
                return EXIT_FAILURE;
            }
        }

        // Create filename string from buffer
        const char *filename = &filename_buffer[0];

        // Print current filename
        printf("Extracting %s from %s...\n", filename, archive_path);

        // Seek to end of filename
        size_t seek = fseek(archive_pointer, position + strlen(filename) + 1, SEEK_SET);

        // Read compressed size
        uint32_t compressed_size;
        if (fread(&compressed_size, 4, 1, archive_pointer) != 1) {
            fclose(archive_pointer);
            fprintf(stderr, "Could not read compressed size\n");
            return EXIT_FAILURE;
        }

        // Read uncompressed size
        uint32_t uncompressed_size;
        if (fread(&uncompressed_size, 4, 1, archive_pointer) != 1) {
            fclose(archive_pointer);
            fprintf(stderr, "Could not read uncompressed size\n");
            return EXIT_FAILURE;
        }

        // Read compression level
        char compression_level;
        if (fread(&compression_level, 1, 1, archive_pointer) != 1) {
            fclose(archive_pointer);
            fprintf(stderr, "Could not read compression level\n");
            return EXIT_FAILURE;
        }

        // Get position in file
        position = ftell(archive_pointer);

        // Read compressed data
        char *compressed_data = malloc(compressed_size);
        if (fread(compressed_data, compressed_size, 1, archive_pointer) != 1) {
            fclose(archive_pointer);
            fprintf(stderr, "Could not read file data\n");
            return EXIT_FAILURE;
        }

        // Create file path
        char *file_path = make_file_path(folder_path, filename);

        // Extract the file
        if (compression_level == 0) {
            // Print warning if compressed size does not match uncompressed size
            if (compressed_size != uncompressed_size) {
                fprintf(stderr, "Compressed size does not match uncompressed size\n");
            }

            // Open file
            FILE *file_pointer = fopen(file_path, "wb");
            free(file_path);
            if (file_pointer == NULL) {
                fclose(archive_pointer);
                fprintf(stderr, "Error creating file\n");
                return EXIT_FAILURE;
            }

            // Write uncompressed data to file
            const int write_status = fwrite(compressed_data, compressed_size, 1, file_pointer);
            fclose(file_pointer);
            free(compressed_data);
            if (write_status != 1) {
                fclose(archive_pointer);
                fprintf(stderr, "Error writing file data\n");
                return EXIT_FAILURE;
            }

        } else if (compression_level == 1) {
            // Perform type 1 decompression
            unsigned int compressed_pointer = 0;
            char *uncompressed_data = malloc(uncompressed_size);
            unsigned int uncompressed_pointer = 0;

            // While there is still compressed data to read
            while (compressed_pointer < compressed_size) {
                // Read flag byte
                const unsigned char flag = compressed_data[compressed_pointer++];

                // Next byte is duplicated x times
                if (flag > 127) {
                    // Calculate times to duplicate
                    const unsigned char count = flag - 125;

                    // Duplicate byte and write to uncompressed data buffer
                    for (unsigned char i = 0; i < count; i++) {
                        uncompressed_data[uncompressed_pointer++] = compressed_data[compressed_pointer];
                    }
                    compressed_pointer++;
                // Next x bytes are copied without duplication
                } else {
                    for (unsigned char i = 0; i < flag + 1; i++) {
                        uncompressed_data[uncompressed_pointer++] = compressed_data[compressed_pointer++];
                    }
                }
            }

            // Free compressed data from memory
            free(compressed_data);

            // Print warning if output size does not match expected size
            if (uncompressed_pointer != uncompressed_size) {
                fprintf(stderr, "Output size does not match expected size\n");
            }

            // Open file
            FILE *file_pointer = fopen(file_path, "wb");
            free(file_path);
            if (file_pointer == NULL) {
                fclose(archive_pointer);
                free(uncompressed_data);
                fprintf(stderr, "Error creating file\n");
                return EXIT_FAILURE;
            }

            // Write uncompressed data to file
            const int write_status = fwrite(uncompressed_data, uncompressed_size, 1, file_pointer);
            fclose(file_pointer);
            free(uncompressed_data);
            if (write_status != 1) {
                fclose(archive_pointer);
                fprintf(stderr, "Error writing file data\n");
                return EXIT_FAILURE;
            }

        } else if (compression_level > 1) {
            // Calculate bits used for offset and run length
            const unsigned char offset_bits = 6 - compression_level;
            if ( offset_bits > 8 ) {
                free(file_path);
                free(compressed_data);
                fprintf(stderr, "Unsupported run and offset length\n");
                continue;
            }
            const unsigned char run_length_bits = 8 - offset_bits;
            const unsigned int max_run_length = (1 << run_length_bits) + 2;
            const int max_offset = (1 << ( offset_bits + 8)) - 1;

            // Create compressed pointer
            uint_fast32_t compressed_pointer = 0;

            // Create uncompressed buffer and pointer
            char *uncompressed_data = malloc(uncompressed_size);
            uint_fast32_t uncompressed_pointer = 0;

            // Create circular buffer
            char *circular_buffer = malloc(max_offset);
            int circular_offset = 0;

            // While there is still compressed data to read
            while (compressed_pointer < compressed_size) {
                // Read flag byte
                const char flag = compressed_data[compressed_pointer++];

                for (unsigned char bit = 0; bit < 8; bit++) {
                    // If data is uncompressed
                    if (((flag >> bit) & 1) == 1) {
                        // Get byte from compressed buffer
                        char *byte = &compressed_data[compressed_pointer++];

                        // Write byte to uncompressed buffer and circular buffer
                        uncompressed_data[uncompressed_pointer++] = *byte;

                        // Write byte to circular buffer
                        circular_buffer[circular_offset] = *byte;
                        if ( circular_offset < max_offset ) {
                            circular_offset++;
                        } else {
                            circular_offset = 0;
                        }

                    // If data is compressed
                    } else {
                        // Get offset and run length
                        int offset = -1;
                        unsigned int count = 0;
                        for (unsigned char i = 0; i < 8; i++, count++) {
                            if (((compressed_data[compressed_pointer] >> i) & 1) == 1) {
                                offset += 1 << count;
                            }
                        }
                        compressed_pointer++;
                        unsigned int run_length = 2;
                        for (unsigned char i = 0; i < 8; i++, count++) {
                            if ( i == offset_bits ) {
                                count = 0;
                            }
                            if (((compressed_data[compressed_pointer] >> i) & 1) == 1) {
                                if ( i >= offset_bits ) {
                                    run_length += 1 << count;
                                } else {
                                    offset += 1 << count;
                                }
                            }
                        }
                        compressed_pointer++;

                        // Check offset is valid
                        if (offset < 0) {
                            printf("'%s' has invalid offset during decompression\n", filename);
                            goto ESCAPE_LOOP;
                        }

                        // Check run length is valid
                        if (run_length > max_run_length) {
                            printf("'%s' has invalid run length during decompression\n", filename);
                            goto ESCAPE_LOOP;
                        }

                        // For each byte in run length
                        for (unsigned int i = 0; i < run_length; i++) {

                            // Check offset is valid
                            if (offset > max_offset || offset >= uncompressed_pointer) {
                                printf("'%s' has invalid offset during decompression\n", filename);
                                goto ESCAPE_LOOP;
                            }

                            // Read byte from circular buffer
                            const char *byte = &circular_buffer[offset];

                            // Write byte to circular buffer
                            circular_buffer[circular_offset] = *byte;
                            if ( circular_offset < max_offset ) {
                                circular_offset++;
                            } else {
                                circular_offset = 0;
                            }

                            // Write byte to uncompressed buffer
                            uncompressed_data[uncompressed_pointer++] = *byte;

                            // Calculate next offset
                            if ( offset < max_offset ) {
                                offset++;
                            } else {
                                offset = 0;
                            }
                        }
                    }
                    if (compressed_pointer >= compressed_size) {
                        break;
                    }
                }
            }

            // Free circular buffer and compressed data from memory
            free(circular_buffer);
            free(compressed_data);

            // Check file matches expected size
            ESCAPE_LOOP:if (uncompressed_pointer != uncompressed_size || compressed_pointer != compressed_size) {
                printf("'%s' does not match expected size\n", filename);
            }

            // Open file
            FILE *file_pointer = fopen(file_path, "wb");
            free(file_path);
            if (file_pointer == NULL) {
                fclose(archive_pointer);
                fprintf(stderr, "Error opening file\n");
                return EXIT_FAILURE;
            }

            // Copy from memory to file
            const int write_status = fwrite(uncompressed_data, uncompressed_size, 1, file_pointer);
            fclose(file_pointer);
            free(uncompressed_data);
            if (write_status != 1) {
                fclose(archive_pointer);
                fprintf(stderr, "Error writing file data\n");
                return EXIT_FAILURE;
            }

        } else {
            fclose(archive_pointer);
            return EXIT_FAILURE;
        }

        // Seek to next file positon
        seek = fseek(archive_pointer, position + compressed_size, SEEK_SET);
        if (seek) {
            fclose(archive_pointer);
            return seek;
        }
    }

    // Close archive and return success code
    fclose(archive_pointer);
    return EXIT_SUCCESS;
}

int pack(const char *folder_path, const char *archive_path) {
    // Open folder
    DIR *folder_pointer = NULL;
    if ((folder_pointer = opendir(folder_path)) == NULL) {
        fprintf(stderr, "Error opening folder %s\n", folder_path);
        return EXIT_FAILURE;
    }

    // Open archive
    FILE *archive_pointer = NULL;
    if ((archive_pointer = fopen(archive_path, "wb")) == NULL) {
        closedir(folder_pointer);
        fprintf(stderr, "Error opening archive %s\n", archive_path);
        return EXIT_FAILURE;
    }

    // For each file in folder
    struct dirent* file_entry;
    while ((file_entry = readdir(folder_pointer))) {
        // Skip . and .. mappings
        if (!strcmp(file_entry->d_name, ".") || !strcmp(file_entry->d_name, "..")) {
            continue;
        }

        // Skip files with long names
        if (strlen(file_entry->d_name) >= FILENAME_SIZE) {
            printf("Skipping file with long filename %s\n", file_entry->d_name);
            continue;
        }

        // Print current filename
        printf("Adding %s to %s...\n", file_entry->d_name, archive_path);

        // Open file
        char *file_path = make_file_path(folder_path, file_entry->d_name);
        FILE *file_pointer = fopen(file_path, "rb");
        free(file_path);
        if (file_pointer == NULL) {
            fprintf(stderr, "Error opening file\n");
            return EXIT_FAILURE;
        }

        // Get file size
        const size_t file_size = get_file_size(file_pointer);
        if (file_size > UINT32_MAX) {
            fprintf(stderr, "File is too large\n");
            free(file_pointer);
            return EXIT_FAILURE;
        }

        // Read file data
        char *file_data = malloc(file_size);
        fread(file_data, file_size, 1, file_pointer);
        fclose(file_pointer);

        // Convert file size to byte array
        char file_size_bytes[4];
        for (int i = 0; i < 4; i++) {
            file_size_bytes[i] = (file_size >> 8 * i) & 0xFF;
        }

        // Create metadata
        const int metadata_size = strlen(file_entry->d_name) + 10;
        char *metadata = malloc(metadata_size);
        strcpy(metadata, file_entry->d_name);
        memcpy(&metadata[metadata_size - 9], &file_size_bytes, 4);
        memcpy(&metadata[metadata_size - 5], &file_size_bytes, 4);
        metadata[metadata_size - 1] = '\0';

        // Write metadata to arhive
        int write_status = fwrite(metadata, metadata_size, 1, archive_pointer);
        free(metadata);
        if (write_status != 1) {
            closedir(folder_pointer);
            fclose(archive_pointer);
            fprintf(stderr, "Error writing metadata to archive\n");
            return EXIT_FAILURE;
        }

        // Write file data to file
        write_status = fwrite(file_data, file_size, 1, archive_pointer);
        free(file_data);
        if (write_status != 1) {
            closedir(folder_pointer);
            fclose(archive_pointer);
            fprintf(stderr, "Error writing file data to archive\n");
            return EXIT_FAILURE;
        }
    }

    // Close folder
    closedir(folder_pointer);

    // Write end of file byte to file
    const char eof_byte[1] = {'\0'};
    fwrite(eof_byte, 1, 1, archive_pointer);

    // Close archive
    fclose(archive_pointer);

    // Reached end, success
    return EXIT_SUCCESS;
}
