#!/bin/bash
echo "----------------------------"
echo -n "Comprimindo:     "
/usr/bin/time -f "%e s / $3" ../test/enc.sh $1 $2 $3 $4 $5

echo -n "Descomprimindo:  "
/usr/bin/time -f "%e s / $3" ../test/dec.sh $1 $2 $3 $4 $5

#echo "Comparando:  "
../test/comp.sh $1 $2 $3 $4 $5
echo "----------------------------"
