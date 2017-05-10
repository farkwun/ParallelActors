#!/bin/sh

END=$1

for ((i=1;i<=END;i++)); do
  ./client &
done
