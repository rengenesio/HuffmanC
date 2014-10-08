#!/bin/bash

for i in `seq 1 $3`; do ../release/$1/encoder$4 in$2.txt out.bin cb.bin; done
