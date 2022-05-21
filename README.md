# file-finder

## Introduction

file-finder is a simple C++ program that searches the provided directory for files with one of the substrings in their name.

    file-finder <dir> <substring1> [<substring2> [<substring3>]...]

Type `dump` during execution to force any matches found to dump to the console. Type `exit` to quit the search.

## Design

file-finder spawns a thread to service each substring. The main thread recursively iterates through the specified directory in a breadth-first strategy. File names are added to the substring worker thread work queues. If a file name contains the substring, it is added to a buffer that dumps to the console every second.

## Compile Instructions

    clang++ --std=c++20 *.cpp -o file-finder

