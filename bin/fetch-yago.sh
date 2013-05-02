#!/bin/bash

DATA_FILE=http://www.mpi-inf.mpg.de/yago-naga/yago/download/yago/yagoFacts.ttl.7z
OUTPUT_FILE=yagoFacts.7z
TARGET_DIR=data

cd `dirname $0`/..

command -v wget >/dev/null 2>&1 || {
  command -v curl >/dev/null 2>&1 || {
    echo >&2 "Neither wget nor curl available. Cannot run."
    exit 1
  }
  dl="curl -C - $DATA_FILE -o $TARGET_DIR/$OUTPUT_FILE"
}

if [ -z "$dl" ]; then
  dl="wget --continue $DATA_FILE -O $TARGET_DIR/$OUTPUT_FILE"
fi

mkdir -p $TARGET_DIR

$dl
7z x -o$TARGET_DIR -y $TARGET_DIR/$OUTPUT_FILE
