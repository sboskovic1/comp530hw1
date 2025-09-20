#!/bin/bash
clear
rm Build/core*
./run.sh
cd /storage-home/s/sb121/comp530/comp530hw1/Build
gdb bin/bufferUnitTest core*