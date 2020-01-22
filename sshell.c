#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define CMDLINE_MAX 512
#define ARGUMENTS_MAX 16
#define TOKEN_MAX 32

typedef struct cmdLineStruct {
	char *arguments[ARGUMENTS_MAX];
} cmdLine;

typedef struct stack {
	int test;
} stack;


void printCompleteMessage(char *completedCommand, int retVal)
{
	fprintf(stderr, "+ completed '%s' [%d]\n", completedCommand, retVal);
}

void executeBuiltIn(char *firstArg, char *entireCommand)
{
	if (!strcmp(firstArg, "pwd")) {
		char cwd[CMDLINE_MAX];
		getcwd( cwd, sizeof(cwd));
		printf("%s\n",cwd);
		printCompleteMessage(firstArg, WEXITSTATUS(0));
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
			printCompleteMessage(entireCommand, 1);
		} else {
			char *returnString = "";
			returnString = strchr(entireCommand, ' ');
			if (returnString == NULL){
				printf("Error: no such directory\n");
				printCompleteMessage(entireCommand, 1);
			} else {
				if (returnString[0] == ' '){
					returnString++;
				}
				checkCd = chdir(returnString);
				/* If checkCd failed, then print out an error message */
				if (checkCd == -1){
					printf("Error: no such directory\n");
					printCompleteMessage(entireCommand, 1);
				} else {
					printCompleteMessage(entireCommand, 0);
				}
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
	free(dest);
	return cmd;
}

char *removeLeadingSpace(char *cmd)
{
	char *firstChar = &cmd[0];
	for (size_t i = 0; i < strlen(cmd); i++) {
		if (!isspace(*firstChar)) {
			break;
		}
		firstChar++;
	}
	return firstChar;
}
int main(void)
{
	char *cmd = (char *)malloc(CMDLINE_MAX);
	char *copyArg = (char *)malloc(CMDLINE_MAX);
	char *firstArg;

	while (1) {
		char *nl;

		/* Print prompt */
		printf("sshell$ ");
		fflush(stdout);

		/* Get command line */
		fgets(cmd, CMDLINE_MAX, stdin);
		cmd = removeLeadingSpace(cmd);

		/* Print command line if stdin is not provided by terminal */
		if (!isatty(STDIN_FILENO)) {
			printf("%s", cmd);
			fflush(stdout);
		}

		/* Remove trailing newline from command line */
		nl = strchr(cmd, '\n');
		if (nl) {
			*nl = '\0';
		}

		/* Created to store the original message for the print
		statement after completion */

		strcpy(copyArg, cmd);

		/* If nothing is input into the command line,
		continue */
		int cmdLen = strlen(cmd);
		if (cmdLen == 0){
			continue;
		}

		/* Check for BuiltIn Command */
		firstArg = returnBeforeSpace(cmd);

		while (1) {
			char *newArg = returnBeforeSpace(removeLeadingSpace(cmd));
			if (strlen(newArg) == 0){
				break;
			}
			printf("new args : %s\n", newArg);
			cmd += strlen(newArg) + 1;
			// add the new arg to struct
		}

		if ((!strcmp(firstArg, "pwd")) || (!strcmp(firstArg, "cd"))
		|| (!strcmp(firstArg, "exit"))) {
			executeBuiltIn(firstArg, copyArg);
		} else if ((!(strcmp(firstArg, cmd)))) {
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
