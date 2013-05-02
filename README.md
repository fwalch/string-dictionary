# string-dictionary

[![Build Status](https://secure.travis-ci.org/fwalch/string-dictionary.png?branch=master)](http://travis-ci.org/fwalch/string-dictionary)

String dictionary implementations for RDF databases

    bin/              Helper scripts and compiled executables
    data/             Sample data
    gtest/            googletest source code
    obj/              Build output
    src/              Source code
    test/             Test source code

## Quickstart

 1. Execute `make` to compile the source code.
 2. Execute `./bin/fetch-yago.sh` to download Yago Facts sample data to `data/yagoFacts.ttl` (optional).
 3. Run `bin/load [turtle file]` to load data into the string dictionary.

To execute tests, run `make test`.

## Requirements

Tested on Arch Linux x64 with Clang 3.2; a sufficiently recent version of GCC should work, too.
For helper scripts, bash, wget/curl and 7zip are used.

## License notices

Contains source code of [`googletest`](https://code.google.com/p/googletest) (see its [`gtest/COPYING`](license terms)).
