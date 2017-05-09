#! /bin/bash

DIR=$PWD

[[ -d patches ]] || mkdir patches

COMMITS=$(git log --reverse --format="format:%H" -- "$DIR" ":!*CMakeLists.txt")

COUNTER=1
for SHA1 in $COMMITS; do

    filename=$(printf "%04d" $COUNTER)-$(git log --format=%f -n 1 ${SHA1}).patch

    # Generate diff
    git diff -u -p ${SHA1}^..${SHA1} -- "$DIR" ":!*CMakeLists.txt" > "patches/${filename}"

    ((COUNTER++))
done
