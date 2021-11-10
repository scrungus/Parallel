all:
	gcc -pthread src/main.c -o main
	./main -P 0.1 -p 3 -n 10 -af e

test: 
	gcc -shared -o main.so -fPIC -pthread src/main.c