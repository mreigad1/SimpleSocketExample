#!/bin/bash
mytitle="Client Chat Window"
echo -e '\033]2;'$mytitle'\007'
./clientSocket $1 $2 $3