#!/bin/bash
pushd build &> /dev/null
	gcc -std=c++11 -fno-exceptions -D DEBUG=1 -D OS_LINUX=1 -o "asteroids" "../src/main.cpp" -l m -l X11 -l GL
popd &> /dev/null