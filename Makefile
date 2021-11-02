all:
	gcc src/main.c -o src/main
	./src/main -p 3 -P 0.1 -n 4 -a {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4}