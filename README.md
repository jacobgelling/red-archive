# Red Archive

A tool for packing and unpacking archives from the 1995 game *Big Red Racing*.

## Usage

The latest binaries for Windows, Debian, and macOS are available on the
[GitHub Releases](https://github.com/jacobgelling/red-archive/releases/latest) page.

To unpack an archive `DIRT1.ENV` into a directory `DIRT1`:

```bash
red-archive -u DIRT1.ENV DIRT1
```

To pack a directory `DIRT1` into an archive `DIRT1.ENV`:

```bash
red-archive -p DIRT1 DIRT1.ENV
```

## Building

Building requires a C compiler and [CMake](https://cmake.org/).

Generate the build files from the repository root:

```bash
cmake . -B build
```

Build the project:

```bash
cmake --build build
```

The output binary can be found in the `bin` directory.

## Format

The *Big Red Racing* archive format and its compression methods are undocumented beyond what is written here.

### Archive

The archive format has no header and does not support directories. Files are stored consecutively with the following
structure:

| Offset  | Bytes | Description                 |
|---------|-------|-----------------------------|
| 0       | *n*   | File name (null-terminated) |
| *n*     | 4     | Compressed size (*m*)       |
| *n* + 4 | 4     | Uncompressed size           |
| *n* + 8 | 1     | Compression type            |
| *n* + 9 | *m*   | File data                   |

### Compression Type 0

Type 0 is uncompressed. File data is stored as-is.

### Compression Type 1

Type 1 is run-length encoding (RLE).

A flag byte *n* determines what follows:

- If *n* < 128, the next *n* bytes are literals.
- If *n* ≥ 128, the following single byte is repeated *n* − 125 times.

### Compression Types 2–6

Types 2–6 use [LZSS](https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski)-style
compression with a sliding window. The compression type *t* determines the bit widths of the offset and run-length
fields: *t* + 8 bits for the offset and 8 − *t* bits for the run length.

| Type | Offset Bits | Length Bits | Window Size | Min Run | Max Run |
|------|-------------|-------------|-------------|---------|---------|
| 2    | 10          | 6           | 1024        | 3       | 66      |
| 3    | 11          | 5           | 2048        | 3       | 34      |
| 4    | 12          | 4           | 4096        | 3       | 18      |
| 5    | 13          | 3           | 8192        | 3       | 10      |
| 6    | 14          | 2           | 16384       | 3       | 6       |

Each flag byte encodes eight chunks. A set bit indicates a compressed chunk and a clear bit indicates an uncompressed
chunk:

- **Uncompressed chunk**: a single literal byte, copied to both the sliding window and the output.
- **Compressed chunk**: two bytes encoding a back-reference offset and run length into the sliding window. The
  referenced bytes are copied to both the sliding window and the output.
