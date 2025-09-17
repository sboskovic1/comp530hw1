#!/bin/bash
clear
gcc sim.c fetch.c issue.c exec.c mem.c write.c sync.c utils.c datahazards.c programs.c ./yacsim.o -lm -o run