/*
 * Red Archive
 * MIT License
 * Copyright (c) 2020 Jacob Gelling
 */

#ifndef REDARCHIVE_ARCHIVE_H
#define REDARCHIVE_ARCHIVE_H

#ifdef _WIN32
    #include <direct.h>
    #include "dirent.h"
#else
    #include <sys/stat.h>
    #include <dirent.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 

int unpack(const char *archive_path, const char *folder_path);
int pack(const char *folder_path, const char *archive_path);

#endif
