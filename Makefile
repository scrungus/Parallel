all:
	gcc -pthread src/main.c -o src/main
	./src/main -P 0.1 -p 5 -n 317 -af e
