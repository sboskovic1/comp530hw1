#!/bin/bash
clear
cd /storage-home/t/tl107/comp432/comp530hw1/Build
if [ -n "$1" ]; then
    bin/bufferUnitTest > "../$1"
else
    bin/bufferUnitTest
fi
