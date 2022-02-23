#!/bin/bash
for ((i=1000;i<=20000;i+=1000)); do
    echo $i
    ./exe $i $i $1 $1
done