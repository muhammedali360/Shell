#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define CMDLINE_MAX 512
#define ARGUMENTS_MAX 16
#define TOKEN_MAX 32

/* Struct to hold cmdline arguments */
typedef struct CmdLineStruct {
	char *arguments[ARGUMENTS_MAX];
} CmdLine;

/* Prints complete message to stderr */
void printCompleteMessage(char *completedCommand, int retVal)
{
	fprintf(stderr, "+ completed '%s' [%d]\n", completedCommand, retVal);
}
/* Handles execution of pwd, exit and cd */
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
		/* Check if cd lacks an argument, if so print an error
		message */
		char *returnString = "";
		returnString = strchr(entireCommand, ' ');
		if (returnString == NULL){
			printf("Error: no such directory\n");
			printCompleteMessage(entireCommand, 1);
		} else {
			/* Increment the pointer after strchr to reach the first character */
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
/* Return the first argument */
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
/* Remove leading white spaces */
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
	char *nl;
	CmdLine structOfArgs;
	int structStart;


	while (1) {

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

		/* Remove leading white spaces in the cmd*/
		cmd = removeLeadingSpace(cmd);

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
		if (!strlen(cmd)){
			continue;
		}

		/* Check for BuiltIn Command */
		firstArg = returnBeforeSpace(cmd);

		/* Parse the string, splitting it by spaces */
		structStart = 0;
		while (1) {
			char *newArg = returnBeforeSpace(removeLeadingSpace(cmd));
			if (strlen(newArg) == 0){
				break;
			}
			cmd += strlen(newArg) + 1;
			structOfArgs.arguments[structStart] = (char *)malloc(CMDLINE_MAX);
			strcpy(structOfArgs.arguments[structStart], newArg);
			structStart++;
		}
		structOfArgs.arguments[structStart] = NULL;

		/* Handles BuiltIn commands */
		if ((!strcmp(firstArg, "pwd")) || (!strcmp(firstArg, "cd"))
		|| (!strcmp(firstArg, "exit"))) {
			executeBuiltIn(firstArg, copyArg);
		/* Execution for non BuiltIn commands */
		} else {
			pid_t pid;
			int status;

			pid = fork();
			if (pid == 0){
			/* Child */
			execvp(structOfArgs.arguments[0],
				structOfArgs.arguments);
			printf("Error: command not found\n");
			exit(1);
			}  else if (pid > 0) {
				/* Parent */
				wait(&status);
				printCompleteMessage(copyArg,
				WEXITSTATUS(status));
			} else {
				perror("fork\n");
				exit(1);
			}
		}
	}
        return EXIT_SUCCESS;
}
