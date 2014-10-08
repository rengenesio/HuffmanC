#!/bin/bash

# Chamar:  ./test.sh versao tamanho_entrada vezes otimizacao profiling
# Exemplo: ./test.sh otimizado 1mb 10 O2 1

if [ ! -e $5 ]; then
	echo "Profiling"
fi

echo "Numero de rodadas: $3"
echo "Comprimindo:"
time for i in `seq 1 $3`; do ../release/$1/encoder$4 in$2.txt out.bin cb.bin; done

if [  ! -e $5 ]; then
	gprof ../release/$1/encoder$4 gmon.out > profiling/$1/encoder$2_$4.txt
fi

echo ""
echo "Descomprimindo:"
time for i in `seq 1 $3`; do ../release/$1/decoder$4 out.bin out.txt cb.bin; done

if [  ! -e $5 ]; then
	gprof ../release/$1/decoder$4 gmon.out > profiling/$1/decoder$2_$4.txt
fi

echo ""
echo "Comparando..."
diff -q in$2.txt out.txt
echo ""
