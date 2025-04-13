#!/bin/bash

if [[ $# -ne 2 ]]; then
    exit 1
fi

dev_file=$1
dev_group=$2

dev_major=$(cat /proc/devices | awk '$2 == "'$dev_group'" { print $1 }')
mknod -m0666 $dev_file c $dev_major 0
