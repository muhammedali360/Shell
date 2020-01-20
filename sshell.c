#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define ARGUMENTS_MAX 16
#define TOKEN_MAX 32

void printCompleteMessage(char *completedCommand, int retVal)
{
	fprintf(stderr, "+ completed '%s' [%d]\n", completedCommand, retVal);
}

int main(void)
{
	char cmd[CMDLINE_MAX];
	pid_t pid;
	int status;

	while (1) {
		char *nl;
                // int retVal;

		/* Print prompt */
		printf("sshell$ ");
		fflush(stdout);

                /* Get command line */
		fgets(cmd, CMDLINE_MAX, stdin);

                // Test strings
		char *args[] = {cmd,NULL};
		char pwdString[256];

                /* Print command line if stdin is not provided by terminal */
		if (!isatty(STDIN_FILENO)) {
                	printf("dog");
                	fflush(stdout);
                }

                /* Remove trailing newline from command line */
		nl = strchr(cmd, '\n');
		if (nl)
			*nl = '\0';

                /* Builtin command */
		if (!strcmp(cmd, "exit")) {
			fprintf(stderr, "Bye...\n");
                  	printCompleteMessage(cmd,0);
                  	exit(0);
		}

                /* Builtin command */

		if (!strcmp(cmd, "pwd")) {
			getcwd(pwdString,256);
			printf("%s\n", pwdString);
			printCompleteMessage(cmd,0);
		}

                /* Regular command */
                // retval = system(cmd);
                // fprintf(stdout, "Return status value for '%s': %d\n",
                //         cmd, retval);
		if(strcmp(cmd, "pwd")){
			pid = fork();
			if (pid == 0) {
				status = execvp(cmd,args);
				perror("execvp");
				exit(1);
			} else if (pid > 0) {
				waitpid(pid,&status,0);
				printCompleteMessage(cmd,status);
				// printf("Child exited with status: %d\n",
				WEXITSTATUS(status);
			} else {
				perror("fork");
				exit(1);
			}
		}
	}
        return EXIT_SUCCESS;
}
