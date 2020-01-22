sshell: sshell.o
	gcc -Wall -Wextra -Werror -o sshell sshell.o
sshell.o: sshell.c
	gcc -Wall -Wextra -Werror -c -o sshell.o sshell.c
debug: sshell.c
	gcc -Wall -Wextra -Werror -g sshell.c -o sshell
clean:
	rm -f sshell sshell.o
