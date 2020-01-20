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

int executeBuiltIn(char *cmd)
{
	if (!strcmp(cmd, "pwd")) {
		pid_t pid;
		int status;
		char *pwdCmd[3] = { "pwd", NULL };
		pid = fork();
		if (pid == 0) {
			status = execvp(cmd, pwdCmd);
			perror("execvp");
			exit(1);
		} else if (pid > 0) {
			wait(&status);
			printCompleteMessage(cmd, WEXITSTATUS(status));
		} else {
			perror("fork");
			exit(1);
		}
	} else if (!strcmp(cmd, "exit")) {
		/* Builtin command */
		fprintf(stderr, "Bye...\n");
		printCompleteMessage(cmd, 0);
		exit(0);
	} else {
		printf("cd\n");
		pid_t pid;
		int status;
		char *cdCmd[3] = { "cd", "" , NULL };
		pid = fork();
		if (pid == 0) {
			status = execvp(cmd, cdCmd);
			perror("execvp");
			exit(1);
		} else if (pid > 0) {
			wait(&status);
			printCompleteMessage(cmd, WEXITSTATUS(status));
		} else {
			perror("fork");
			exit(1);
		}
	}
	return -1;
}
char *returnBeforeSpace(char *cmd)
{
	int stringLength = strlen(cmd);

	printf("String length is %d\n", stringLength);
	printf("cmd is %s\n", cmd);

	int returnLength = 0;
	char *dest = (char *)malloc(stringLength + 1);

	for(int i = 0; i < stringLength; i++){
		if (cmd[i] == ' '){
			returnLength = i;
			strncpy(dest, cmd, returnLength);
			return dest;
		}
	}
	return cmd;
}

int main(void)
{
	char cmd[CMDLINE_MAX];
	// char checkSpace = ' ';
	// char *returnString;
	char *firstArg;

	while (1) {
		char *nl;
		// int retVal;

		/* Print prompt */
		printf("sshell$ ");
		fflush(stdout);

		/* Get command line */
		fgets(cmd, CMDLINE_MAX, stdin);

		/* Print command line if stdin is not provided by terminal */
		if (!isatty(STDIN_FILENO)) {
			printf("%s", cmd);
			fflush(stdout);
		}

		/* Remove trailing newline from command line */
		nl = strchr(cmd, '\n');
		if (nl)
			*nl = '\0';

		/* Builtin command */
		firstArg = returnBeforeSpace(cmd);
		printf("firstArg is |%s| and cmd is |%s|\n", firstArg, cmd);

		if ((!strcmp(cmd, "pwd")) || (!strcmp(firstArg, "cd"))
		|| (!strcmp(cmd, "exit"))) {
			executeBuiltIn(cmd, );
		}

		// firstArg = returnBeforeSpace(cmd);
		// returnString = strchr(cmd, checkSpace);
		//
		// printf("String before |%c| is |%s|\n", checkSpace, firstArg);
		// printf("String after |%c| is |%s|\n", checkSpace, returnString);
	}
        return EXIT_SUCCESS;
}
