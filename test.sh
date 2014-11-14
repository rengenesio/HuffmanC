#!/bin/bash

##############################
#./test.sh n_amostras
##############################
declare path_bin="./release"
declare bin="huffmanC-O3"
declare -a version=("normal" "optimized")
declare path_file="/home/admin/Workspace/Files"
declare -a file=("0000001mb" "0000008mb" "0000064mb" "0000128mb" "0000256mb" "0000512mb" "0001024mb" "0002048mb")

for i in "${version[@]}"
do
	echo "=========================================="
	echo "$i"
	echo "=========================================="

	for j in "${file[@]}"
	do
		echo "$j"
		echo "-------------------------"
		for k in `seq 1 $1`
		do
			echo "Amostra $k"
			$path_bin/$i/$bin $path_file/$j		
			echo
		done
		echo "-------------------------"
		echo
		echo
	done
	echo
done
