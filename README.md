# Red Archive
Program to handle archives from the 1995 title Big Red Racing.

## Usage
Binaries for Windows and Debian are available in [GitHub Releases](https://github.com/jacobgelling/red-archive/releases).

To unpack a given archive `DIRT1.ENV` to folder `DIRT1`, simply type:
```console
$ red-archive -u "DIRT1.ENV" "DIRT1"
```

To pack a given folder `DIRT1` into archive `DIRT1.ENV`, simply type:
```console
$ red-archive -p "DIRT1" "DIRT1.ENV"
```

## Format
The Big Red Racing archive format and compression methods are unidentified and therefore undocumented other than what's written here.

### Archive
The archive format has no header like is typical with other formats, and does not support directories. Each file is stored consecutively in the archive, with a structure as shown below.

| Offset   | Bytes | Description                 |
| -------- | ----- | --------------------------- |
| 0        | *n*   | File name (null-terminated) |
| *n*      | 4     | Compressed size (*m*)       |
| *n* + 4  | 4     | Uncompressed size           |
| *n* + 8  | 2     | Compression type            |
| *n* + 10 | *m*   | File data                   |

### Compression Type 1
Type 1 is a very simple compression method suitable for compressing small files.

### Compression Type 2
Type 2 is a more complicated compression method capable of better compression in large files.
