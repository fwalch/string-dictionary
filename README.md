# string-dictionary

[![Build Status](https://secure.travis-ci.org/fwalch/string-dictionary.png?branch=master)](http://travis-ci.org/fwalch/string-dictionary)

String dictionary implementations for RDF databases

    bin/              Helper scripts and compiled executables
    data/             Sample data
    gmock/            googlemock source code
    gtest/            googletest source code
    obj/              Build output
    src/              Source code
    test/             Test source code

## Quickstart

 1. Execute `make` to compile the source code.
 2. Execute `./bin/fetch-yago.sh` to download Yago Facts sample data to `data/yagoFacts.ttl`.
 3. Run `bin/load [dictionary implementation] [turtle file]` to load some data into the string dictionary.
 4. Wait until loading is finished and measure memory usage using `ps -C load -o rss=`.

To execute tests, run `make test`.

## Requirements

Tested on Arch Linux x64 with Clang 3.2; a sufficiently recent version of GCC should work, too.
For the helper scripts, the following programs are used: bash, 7z, and wget or curl.

## License notices

Contains source code of [`googletest`](https://code.google.com/p/googletest) ([license terms](gtest/COPYING))
and [`googlemock`](https://code.google.com/p/googlemock) ([license terms](gmock/COPYING)).
