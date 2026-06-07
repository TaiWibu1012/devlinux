#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Error: Thieu tham so " 
    exit 1
fi

NAME=$1
AGE=$2

echo "Hello, my name is $NAME and I am $AGE years old."
echo "Total arguments received: $#"
