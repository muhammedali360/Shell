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
	char **placeHolder;
} cmdLine;


void printCompleteMessage(char *completedCommand, int retVal)
{
	fprintf(stderr, "+ completed '%s' [%d]\n", completedCommand, retVal);
}

void executeBuiltIn(char *firstArg, char *entireCommand)
{
	if (!strcmp(firstArg, "pwd")) {
		char cwd[512];
		getcwd( cwd, sizeof(cwd));
		printf("%s\n",cwd);
		printCompleteMessage(firstArg,WEXITSTATUS(0));
	} else if (!strcmp(firstArg, "exit")) {
		/* Builtin command */
		fprintf(stderr, "Bye...\n");
		printCompleteMessage(firstArg, 0);
		exit(0);
	} else {
		int checkCd;
		/* Use this to prevent seg fault if to see if the entireCommand passed is simply
		cd. If so, we print an error message */
		if(!strcmp(firstArg, entireCommand)){
			printf("Error: no such directory\n");
			printCompleteMessage(firstArg, 1);
		} else {
			char *returnString = "";
			returnString = strchr(entireCommand, ' ');
			if (returnString[0] == ' '){
				returnString++;
			}
			checkCd = chdir(returnString);
			if (checkCd == -1){
				printf("Error: no such directory\n");
				printCompleteMessage(firstArg, 1);
			} else {
				printCompleteMessage(firstArg, 0);
			}
		}
	}
}
char *returnBeforeSpace(char *cmd)
{
	int stringLength = strlen(cmd);
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
	char *firstArg;

	while (1) {
		char *nl;
		// int retVal;

		/* Print prompt */
		printf("sshell$ ");
		fflush(stdout);

		/* Get command line */
		fgets(cmd, CMDLINE_MAX, stdin);
		if (strlen(cmd) == 1){
			continue;
		}

		/* Print command line if stdin is not provided by terminal */
		if (!isatty(STDIN_FILENO)) {
			printf("%s", cmd);
			fflush(stdout);
		}

		/* Remove trailing newline from command line */
		nl = strchr(cmd, '\n');
		if (nl)
			*nl = '\0';

		/* Check for BuiltIn Command */
		firstArg = returnBeforeSpace(cmd);
		if ((!strcmp(firstArg, "pwd"))|| (!strcmp(firstArg, "cd"))
		|| (!strcmp(firstArg, "exit"))) {
			executeBuiltIn(firstArg, cmd);
		} else {
			pid_t pid;
			int status;
			char *cmdArgs[2] = { cmd, NULL};

			pid = fork();
			if (pid == 0){
			/* Child */
			execvp(cmdArgs[0],cmdArgs);
			printf("Error: command not found\n");
			exit(1);
			}  else if (pid > 0) {
				/* Parent */
				wait(&status);
				printCompleteMessage(cmd,
				WEXITSTATUS(status));
			} else {
				perror("fork\n");
				exit(1);
			}
		}
	}
        return EXIT_SUCCESS;
}
