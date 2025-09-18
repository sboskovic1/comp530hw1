#!/bin/bash
clear
cd /storage-home/s/sb121/comp530/comp530hw1/Build
echo 2 | ~/scons/bin/scons-3.1.2
if [ -n "$1" ]; then
    bin/bufferUnitTest > "../$1"
else
    bin/bufferUnitTest
fi