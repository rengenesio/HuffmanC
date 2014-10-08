#!/bin/bash

for i in `seq 1 $3`; do ../release/$1/decoder$4 out.bin out.txt cb.bin; done
