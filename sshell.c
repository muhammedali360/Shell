#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define ARGUMENTS_MAX 16
#define TOKEN_MAX 32

typedef struct cmdLineStruct {
	char *arguments[ARGUMENTS_MAX];
} cmdLine;

void printCompleteMessage(char *completedCommand, int retVal)
{
	fprintf(stderr, "+ completed '%s' [%d]\n", completedCommand, retVal);
}

void executeBuiltIn(char *firstArg)
{
	if (!strcmp(firstArg, "pwd")) {
		printf("pwd");
	} else if (!strcmp(firstArg, "exit")) {
		/* Builtin command */
		fprintf(stderr, "Bye...\n");
		printCompleteMessage(firstArg,0);
		exit(0);
	} else {
		printf("cd");
	}
}
char *returnBeforeSpace(char *cmd)
{
	int stringLength = strlen(cmd);
	int returnLength = 0;
	char *dest;

	dest = (char *)malloc(stringLength+1);

	for(int i = 0; i < stringLength; i++){
		if (cmd[i] == ' '){
			returnLength = i;
		}
	}
	strncpy(dest, cmd, returnLength);
	return dest;
}

int main(void)
{
	char cmd[CMDLINE_MAX];
	pid_t pid;
	int status;
	char checkSpace = ' ';
	char *returnString;
	char *firstArg;

	while (1) {
		char *nl;
		// int retVal;

		/* Print prompt */
		printf("sshell$ ");
		fflush(stdout);

		/* Get command line */
		fgets(cmd, CMDLINE_MAX, stdin);

		firstArg = returnBeforeSpace(cmd);
		if ((!strcmp(cmd, "pwd")) || (!strcmp(cmd, "cd"))
		|| (!strcmp(cmd, "exit"))){
			executeBuiltIn(cmd);
		}

		returnString = strchr(cmd, checkSpace);

		printf("String before |%c| is |%s|\n", checkSpace, firstArg);
		printf("String after |%c| is |%s|\n", checkSpace, returnString);

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

		// /* Builtin command */
		// if (!strcmp(cmd, "exit")) {
		// 	fprintf(stderr, "Bye...\n");
		// 	printCompleteMessage(cmd,0);
		// 	exit(0);
		// }

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
