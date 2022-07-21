#!/bin/sh

set -xe
cc="/usr/bin/gcc"
cflags="-Wall -Wextra -std=c99 -pedantic -ggdb"
libs="`pkg-config --cflags --libs sdl2` -lm"
src=( $(ls *.c) )
out="rogueban"
$cc $cflags -c ${src[*]}
objs=( $(ls *.o) )
$cc ${objs[*]} $libs -o $out
set +xe
