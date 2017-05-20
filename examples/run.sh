#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../install/lib
gcc -I ../install/include -L ../install/lib/ -o hello_world hello_world.c -lcam -lpthread
./hello_world
../install/bin/cam -p
