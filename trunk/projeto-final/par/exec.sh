if [ $# -ne 2 ]; then
	echo "Falta argumento. Use: sh exec.sh <num_blocos> <num_threads>"
else
	echo "Filtrando o arquivo...OK!"
	./filter palavras.txt fim.txt
	./main fim.txt $1 $2
	rm fim.txt
fi
