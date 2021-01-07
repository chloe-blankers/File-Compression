# File-Compression
Implemented Lempel-Ziv-Welch algorithm (lossless data compression algorithm) in C to compress and decompress text files by at least 50%.

## Dictionary Data Structure
The dictionary data structure to be used has a capacity of 4096 entries,
of which:
- the first 256 entries, numbered 0 through 255, are reserved for every possible 1-byte value
- the entries numbered 256 through 4094 are assigned dynamically by the compression
algorithm to represent multi-byte sequences in the input data.
- the last entry, numbered 4095, is reserved for a padding marker
The code that refers to an entry in the dictionary is simply the entry’s numeric position, encoded
as a 12-bit binary integer.
