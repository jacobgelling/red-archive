/*
 * Red Archive
 * MIT License
 * Copyright (c) 2020 Jacob Gelling
 */

#include "archive.h"

// Set filename size to 13 (length of 8.3 filename with null-terminator)
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

int unpack(const char *archive_path, const char *folder_path) {
    // Open archive
    FILE *archive_pointer = NULL;
    #ifdef _WIN32
        int archive_open = fopen_s(&archive_pointer, archive_path, "rb");
        if (archive_open) {
            fclose(archive_pointer);
            return archive_open;
        }
    #else
        if ((archive_pointer = fopen(archive_path, "rb")) == NULL) {
            fclose(archive_pointer);
            return 1;
        }
    #endif

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
        unsigned int compressed_size;
        if (fread(&compressed_size, 4, 1, archive_pointer) != 1) {
            fclose(archive_pointer);
            return 1;
        }

        // Read uncompressed size
        unsigned int uncompressed_size;
        if (fread(&uncompressed_size, 4, 1, archive_pointer) != 1) {
            fclose(archive_pointer);
            return 1;
        }

        // Read compression level
        char compression_level;
        if (fread(&compression_level, 1, 1, archive_pointer) != 1) {
            fclose(archive_pointer);
            return 1;
        }

        // Get position in file
        position = ftell(archive_pointer);

        // Read compressed data
        char *compressed_data = malloc(compressed_size);
        fread(compressed_data, compressed_size, 1, archive_pointer);

        // Create file path
        const size_t file_path_size = strlen(filename) + strlen(folder_path) + 2;
        char *file_path = malloc(file_path_size);
        #ifdef _WIN32
            strcpy_s(file_path, file_path_size, folder_path);
            strcat_s(file_path, file_path_size, "/");
            strcat_s(file_path, file_path_size, filename);
        #else
            strcpy(file_path, folder_path);
            strcat(file_path, "/");
            strcat(file_path, filename);
        #endif

        // Extract the file
        if (compression_level == 0) {
            if (compressed_size != uncompressed_size) {
                printf("'%s' does not match expected size\n", filename);
            }

            // Copy from memory to file
            FILE *file_pointer = NULL;
            #ifdef _WIN32
                int file_open = fopen_s(&file_pointer, file_path, "wb");
                if (file_open) {
                    fclose(file_pointer);
                    return file_open;
                }
            #else
                if ((file_pointer = fopen(file_path, "wb")) == NULL) {
                    fclose(file_pointer);
                    return 1;
                }
            #endif
            fwrite(compressed_data, compressed_size, 1, file_pointer);
            fclose(file_pointer);

        } else if (compression_level == 1) {
            // Perform type 1 decompression
            unsigned int compressed_pointer = 0;
            char *uncompressed_data = malloc(uncompressed_size);
            unsigned int uncompressed_pointer = 0;

            while (compressed_pointer < compressed_size) {
                unsigned char flag = compressed_data[compressed_pointer];
                compressed_pointer++;
                if (flag > 127) {
                    // Next byte is duplicated x times
                    char byte = compressed_data[compressed_pointer];
                    compressed_pointer++;
                    unsigned char count = flag - 125;
                    for (unsigned char i = 0; i < count; i++) {
                        uncompressed_data[uncompressed_pointer] = byte;
                        uncompressed_pointer++;
                    }
                } else {
                    // Next x bytes are copied without duplication
                    for (unsigned char i = 0; i < flag + 1; i++) {
                        uncompressed_data[uncompressed_pointer] = compressed_data[compressed_pointer];
                        uncompressed_pointer++;
                        compressed_pointer++;
                    }
                }
            }

            // Check uncompressed file matches expected size
            if (uncompressed_pointer != uncompressed_size) {
                printf("'%s' does not match expected size\n", filename);
            }

            // Copy from memory to file
            FILE *file_pointer = NULL;
            #ifdef _WIN32
                int file_open = fopen_s(&file_pointer, file_path, "wb");
                if (file_open) {
                    fclose(file_pointer);
                    return file_open;
                }
            #else
                if ((file_pointer = fopen(file_path, "wb")) == NULL) {
                    fclose(file_pointer);
                    return 1;
                }
            #endif
            fwrite(uncompressed_data, uncompressed_size, 1, file_pointer);
            fclose(file_pointer);

            // Free uncompressed data from memory
            free(uncompressed_data);

        } else if (compression_level > 1) {
            // Calculate bits used for offset and run length
            const unsigned char offset_bits = 6 - compression_level;
            if ( offset_bits > 8 ) {
                printf("'%s' has unsupported run and offset length\n", filename);
                free(file_path);
                free(compressed_data);
                continue;
            }
            const unsigned char run_length_bits = 8 - offset_bits;
            const unsigned int max_run_length = (1 << run_length_bits) + 2;
            const int max_offset = ( 1 << ( offset_bits + 8 ) ) - 1;

            // Create compressed buffer
            unsigned int compressed_pointer = 0;

            // Create uncompressed buffer
            char *uncompressed_data = malloc(uncompressed_size);
            unsigned int uncompressed_pointer = 0;

            // Create sliding window
            char *sliding_window = malloc(max_offset);
            int sliding_offset = 0;

            while (compressed_pointer < compressed_size) {

                // Get flag
                const char flag = compressed_data[compressed_pointer];
                compressed_pointer++;

                for (unsigned char bit = 0; bit < 8; bit++) {
                    // If data is uncompressed
                    if ( ( (flag >> bit) & 1 ) == 1 ) {
                        // Get byte from compressed buffer
                        char *byte = &compressed_data[compressed_pointer];
                        compressed_pointer++;

                        // Write byte to uncompressed buffer and sliding window
                        uncompressed_data[uncompressed_pointer] = *byte;
                        uncompressed_pointer++;

                        // Write byte to sliding window
                        sliding_window[sliding_offset] = *byte;
                        if ( sliding_offset < max_offset ) {
                            ++sliding_offset;
                        } else {
                            sliding_offset = 0;
                        }

                    // If data is compressed
                    } else {
                        // Get offset and run length
                        int offset = -1;
                        unsigned int count = 0;
                        for (unsigned char i = 0; i < 8; i++) {
                            if ( ( (compressed_data[compressed_pointer] >> i) & 1 ) == 1 ) {
                                offset += 1 << count;
                            }
                            count++;
                        }
                        compressed_pointer++;
                        unsigned int run_length = 2;
                        for (unsigned char i = 0; i < 8; i++) {
                            if ( i == offset_bits ) {
                                count = 0;
                            }
                            if ( ( (compressed_data[compressed_pointer] >> i) & 1 ) == 1 ) {
                                if ( i >= offset_bits ) {
                                    run_length += 1 << count;
                                } else {
                                    offset += 1 << count;
                                }
                            }
                            count++;
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

                            // Read byte from sliding window
                            char *byte = &sliding_window[offset];

                            // Write byte to sliding window
                            sliding_window[sliding_offset] = *byte;
                            if ( sliding_offset < max_offset ) {
                                ++sliding_offset;
                            } else {
                                sliding_offset = 0;
                            }

                            // Write byte to uncompressed buffer
                            uncompressed_data[uncompressed_pointer] = *byte;
                            uncompressed_pointer++;

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

            free(sliding_window);

            // Check file matches expected size
            ESCAPE_LOOP:if (uncompressed_pointer != uncompressed_size || compressed_pointer != compressed_size) {
                printf("'%s' does not match expected size\n", filename);
            }

            // Copy from memory to file
            FILE *file_pointer = NULL;
            #ifdef _WIN32
                int file_open = fopen_s(&file_pointer, file_path, "wb");
                if (file_open) {
                    fclose(file_pointer);
                    return file_open;
                }
            #else
                if ((file_pointer = fopen(file_path, "wb")) == NULL) {
                    fclose(file_pointer);
                    return 1;
                }
            #endif
            fwrite(uncompressed_data, uncompressed_size, 1, file_pointer);
            fclose(file_pointer);

            // Free uncompressed data from memory
            free(uncompressed_data);

        } else {
            fclose(archive_pointer);
            return 1;
        }

        // Free file path and compressed data from memory
        free(file_path);
        free(compressed_data);

        // Seek to next file positon
        seek = fseek(archive_pointer, position + compressed_size, SEEK_SET);
        if (seek) {
            fclose(archive_pointer);
            return seek;
        }
    }

    // Reached end, success
    fclose(archive_pointer);
    return 0;
}

int pack(const char *folder_path, const char *archive_path) {
    // Open folder
    DIR *folder_pointer = opendir(folder_path);
    if (folder_pointer == NULL) {
        closedir(folder_pointer);
        return 1;
    }

    // Open archive
    FILE *archive_pointer = NULL;
    #ifdef _WIN32
        int archive_open = fopen_s(&archive_pointer, archive_path, "wb");
        if (archive_open) {
            fclose(archive_pointer);
            return archive_open;
        }
    #else
        if ((archive_pointer = fopen(archive_path, "wb")) == NULL) {
            fclose(archive_pointer);
            return 1;
        }
    #endif

    // For each file in folder
    struct dirent* file_entry;
    while ((file_entry = readdir(folder_pointer))) {
        // Skip . and .. files
        if (!strcmp (file_entry->d_name, ".") || !strcmp (file_entry->d_name, "..")) {
            continue;
        }

        // Create file path
        const int filename_size = strlen(file_entry->d_name) + strlen(folder_path) + 2;
        char *file_path = malloc(filename_size);
        #ifdef _WIN32
            strcpy_s(file_path, filename_size, folder_path);
            strcat_s(file_path, filename_size, "/");
            strcat_s(file_path, filename_size, file_entry->d_name);
        #else
            strcpy(file_path, folder_path);
            strcat(file_path, "/");
            strcat(file_path, file_entry->d_name);
        #endif

        // Read file data
        FILE *file_pointer = NULL;
        #ifdef _WIN32
            int file_open = fopen_s(&file_pointer, file_path, "wb");
            if (file_open) {
                fclose(file_pointer);
                return file_open;
            }
        #else
            if ((file_pointer = fopen(file_path, "wb")) == NULL) {
                fclose(file_pointer);
                return 1;
            }
        #endif
        fseek(file_pointer, 0, SEEK_END);
        unsigned int file_size = ftell(file_pointer);
        fseek(file_pointer, 0, SEEK_SET);
        char *file_data = malloc(file_size);
        fread(file_data, file_size, 1, file_pointer);
        fclose(file_pointer);

        // Convert file size to bytes
        char file_size_bytes[4];
        file_size_bytes[3] = ((unsigned int) file_size >> 24) & 0xFF;
        file_size_bytes[2] = ((unsigned int) file_size >> 16) & 0xFF;
        file_size_bytes[1] = ((unsigned int) file_size >> 8) & 0xFF;
        file_size_bytes[0] = (unsigned int) file_size & 0xFF;

        // Create metadata
        const int metadata_size = strlen(file_entry->d_name) + 10;
        char *metadata = malloc(metadata_size);
        #ifdef _WIN32
            strcpy_s(metadata, metadata_size, file_entry->d_name);
        #else
            strcpy(metadata, file_entry->d_name);
        #endif
        memcpy(&metadata[metadata_size - 9], &file_size_bytes, 4);
        memcpy(&metadata[metadata_size - 5], &file_size_bytes, 4);
        metadata[metadata_size - 1] = '\0';

        // Copy from memory to file
        fwrite(metadata, metadata_size, 1, archive_pointer);
        fwrite(file_data, file_size, 1, archive_pointer);

        // Free memory
        free(metadata);
        free(file_data);
    }

    // Close archive and folder
    const char eof_byte[1] = {'\0'};
    fwrite(eof_byte, 1, 1, archive_pointer);
    fclose(archive_pointer);
    closedir(folder_pointer);

    // Reached end, success
    return 0;
}
