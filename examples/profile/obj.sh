#!/bin/sh
/home/kku/ispc/ispc --profile --target=sse4-x2 --arch=x86-64 $1 -o $1.o
