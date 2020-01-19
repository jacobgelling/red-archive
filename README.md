# Red Archive
Program to handle archives from the 1995 title Big Red Racing.

## Usage
The latest binaries for Windows and Debian are available at [GitHub Releases](https://github.com/jacobgelling/red-archive/releases/latest).

To unpack a given archive `DIRT1.ENV` to folder `DIRT1`, execute the following.
```bash
red-archive -u DIRT1.ENV DIRT1
```

To pack a given folder `DIRT1` into archive `DIRT1.ENV`, execute the following.
```bash
red-archive -p DIRT1 DIRT1.ENV
```

## Compilation
Compilation requires a C compiler and CMake.

To generate the build files, execute the following from the root project folder.
```bash
cmake . -B build
```

To build the project, execute the following from the root project folder.
```bash
cmake --build build
```

You can find the output binaries in the `bin` folder.

## Format
The Big Red Racing archive format and compression methods are unidentified and therefore undocumented other than what's written here.

### Archive
The archive format has no header like is typical with other formats, and does not support directories. Each file is stored consecutively in the archive, with a structure as shown below.

| Offset   | Bytes | Description                 |
| -------- | ----- | --------------------------- |
| 0        | *n*   | File name (null-terminated) |
| *n*      | 4     | Compressed size (*m*)       |
| *n* + 4  | 4     | Uncompressed size           |
| *n* + 8  | 1     | Compression type            |
| *n* + 9 | *m*   | File data                    |

### Compression Type 0
Type 0 indicates an uncompressed file.

### Compression Type 1
Type 1 is a very simple compression method suitable for compressing small files.

A flag byte *n* indicates whether the next *n* bytes are copied as-is (where *n* < 128), or the following byte is repeated *m* times (where *m* = *n* - 125).

### Compression Type 2
Type 2 is a more complicated compression method capable of better compression in large files.

It has a compression type value ranging from 2 to 6. The size in bits for the run length and offset are calculated from the compression type value.

The bits in a flag byte represent whether the following 8 chunks are compressed or uncompressed.

If a chunk is uncompressed, it is a single uncompressed byte and is copied to the round buffer and output buffer.

If a chunk is compressed, it consists of two bytes which, on a bit-level as dictated by the compression type value, represent the offset in the round buffer and run length. These bytes are then copied to the round buffer and output buffer.
