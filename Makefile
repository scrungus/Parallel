all:
	gcc -pthread src/main.c -o src/main
	./src/main -p 6 -P 0.1 -n 300 -af no.txt
