#!/bin/bash
mytitle="Server Activity Window"
echo -e '\033]2;'$mytitle'\007'
./server 
#> serverDebug.txt 2>&1