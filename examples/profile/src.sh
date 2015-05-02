#!/bin/sh
/home/kku/ispc/ispc --profile --target=generic-4 --arch=x86-64 --emit-c++ $1 -o $1.cpp
