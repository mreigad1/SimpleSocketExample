#!/bin/bash
if [ ! -z "$1" ] ; then
	echo $1 > server.conf
	exit
fi
echo "bob"
