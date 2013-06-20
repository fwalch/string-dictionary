# string-dictionary

[![Build Status](https://secure.travis-ci.org/fwalch/string-dictionary.png?branch=master)](http://travis-ci.org/fwalch/string-dictionary)

String dictionary implementations for RDF databases

    bin/              Helper scripts and compiled executables
    boost/            boost library source code
    btree/            cpp-btree library source code
    b+tree/           stx-btree library source code
    data/             Sample data
    gmock/            googlemock library source code
    gtest/            googletest library source code
    obj/              Build output
    src/              Main source directory
    test/             Unit tests
    tools/            Utility programs

## Quickstart

 1. Execute `make` to compile the source code.
 2. Execute `./bin/fetch-yago.sh` to download Yago Facts sample data to `data/yagoFacts.ttl`.
 3. Run `bin/perftest data/yagoFacts.ttl` to execute performance test for the different string dictionary implementations.

To execute the unit tests, run `make test`.

## Requirements

Tested on Arch Linux x64 with Clang 3.3 and GCC 4.8.1. Windows is not supported, though other platforms might work.
For the helper scripts, the following programs are used: bash, 7z, and wget or curl.

## License notices

Contains modified Adaptive Radix Tree source code by [Viktor Leis](http://www-db.in.tum.de/~leis/).

Also uses source code of [`cpp-btree`](https://code.google.com/p/cpp-btree) ([license terms](btree/COPYING)), [`stx-btree`](http://panthema.net/2007/stx-btree/) ([license terms](b+tree/LICENSE_1_0.txt)) and [Boost](http://www.boost.org) ([license terms](boost/LICENSE_1_0.txt)).

Includes source code of [`googletest`](https://code.google.com/p/googletest) ([license terms](gtest/COPYING)) and [`googlemock`](https://code.google.com/p/googlemock) ([license terms](gmock/COPYING)) for the unit tests.
