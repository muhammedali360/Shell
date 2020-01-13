simpleShell: main.o
	gcc -Wall -Wextra -Werror -o simpleShell main.o
main.o: main.c
	gcc -Wall -Wextra -Werror -c -o main.o main.c
clean:
	rm -f simpleShell main.o 
