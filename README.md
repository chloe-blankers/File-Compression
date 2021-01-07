# File Compression Program
Implemented Lempel-Ziv-Welch algorithm (lossless data compression algorithm) in C to compress and decompress text files by at least 50%. LZW is a method for adaptive / dynamic compression. It starts with an initial model, reads data piece by piece and then updates the model while encoding the data as it proceeds through the input file. LZW can be used for binary and text files (or any other type of file), that said, it is known for performing well on files or data streams with (any type of) repeated substrings.

## Dictionary Data Structure
The dictionary data structure to be used has a capacity of 4096 entries,
of which:
- the first 256 entries, numbered 0 through 255, are reserved for every possible 1-byte value
- the entries numbered 256 through 4094 are assigned dynamically by the compression
algorithm to represent multi-byte sequences in the input data.
- the last entry, numbered 4095, is reserved for a padding marker
The code that refers to an entry in the dictionary is simply the entry’s numeric position, encoded
as a 12-bit binary integer.

## Command
`LZW {input_file} [e | d]`</br>
`e` : encode</br>
`d` : decode
