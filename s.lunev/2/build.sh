#!/bin/bash

all:
    clang -o b2.bin 2.c
    ./b2.bin
