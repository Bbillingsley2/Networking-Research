#!/bin/bash

while true
do
    echo "Running client"
    time ./qdclient 2> log-$(date +%s).csv
done
