#ifndef RED_ARCHIVE_ARCHIVE_H
#define RED_ARCHIVE_ARCHIVE_H

#ifdef _WIN32
#include "dirent.h"
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int unpack(const char *archive_path, const char *folder_path);
int pack(const char *folder_path, const char *archive_path);

#endif
