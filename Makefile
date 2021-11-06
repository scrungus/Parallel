all:
	gcc -pthread src/main.c -o src/main
	./src/main -P 0.1 -p 3 -n 10 -af src/e
