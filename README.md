# LZZ3

######_An LZ77-based data compression algorithm_

## Introduction

This compression algorithm is implemented based on LZ77 (Lempel-Ziv in 1977) and huffman encoding.
The implementation is rather simple, with a practical compression rate and speed.

## Usage

### Windows Executable

Run the following
```
lzz3.exe <mode> <in_file> [-l level] [-o out_file]
```
Where the following `mode`'s are supported:
* `-c` compress
* `-u` uncompress

And the `-l` flag is available in the `-c` mod, followed by a number ranged from `1` to `9`. Larger number of level 
represents higher compression ratio but usually slower in speed.


## Authors

* Bohan Zhang  - _Initial work_ - GitHub: ZhangBohan233
