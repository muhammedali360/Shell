#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define CMDLINE_MAX 512
#define ARGUMENTS_MAX 16
#define TOKEN_MAX 32

/* Prints complete message to stderr */
void printCompleteMessage(char *completedCommand, int retVal)
{
	fprintf(stderr, "+ completed '%s' [%d]\n", completedCommand, retVal);
}

/* Struct to hold cmdline arguments */
typedef struct CmdLineStruct {
	char *arguments[ARGUMENTS_MAX];
} CmdLine;

/* Creating built in stack */

/*Inspired by: https://www.geeksforgeeks.org/stack-data-structure-introduction-program */
typedef struct DirStack
{
	char directory[CMDLINE_MAX];
	struct DirStack* next;
} DirStack;

DirStack *newNode(char *directory)
{
	DirStack* node = (DirStack*)malloc(sizeof(DirStack));
	strcpy(node->directory, directory);
	node->next = NULL;
	return node;
}
int isEmpty(DirStack *root)
{
	return !root;
}

void pushd(DirStack **root, char *directoryToCd, char *entireCommand)
{
	//malloc here
	DirStack *stackNode = newNode(directoryToCd);
	stackNode->next = *root;
	*root = stackNode;
	printf("%s pushed to stack\n", directoryToCd);

	char cwd[CMDLINE_MAX];
	getcwd(cwd, sizeof(cwd));
	int checkCd = chdir(cwd);
	/* If checkCd failed, then print out an error message */
	if (checkCd == -1){
		printf("Error: no such directory\n");
		printCompleteMessage(entireCommand, 1);
	} else {
		printCompleteMessage(entireCommand, 0);
	}
}

// void popd(DirStack **root)
void popd(DirStack **root)
{
	if (isEmpty(*root)){
		printf("Error: directory stack empty\n");
		printCompleteMessage("popd", 1);
		return ;
	}
	DirStack *temp = *root;
	*root = (*root)->next;

	char *poppedDirectory = (char *)malloc(CMDLINE_MAX);
	strcpy(poppedDirectory, temp->directory);
	chdir(poppedDirectory);
	free(temp);
	free(poppedDirectory);
}
char *peek(DirStack *root)
{
	if (isEmpty(root)) {
		printf("Is empty\n");
		return NULL;
	}
	return root->directory;
}

void dirs(DirStack *stack)
{
	char cwd[CMDLINE_MAX];
	getcwd(cwd, sizeof(cwd));
	printf("%s\n",cwd);
	DirStack *temp = stack;
	while (temp != NULL) {
		printf("%s\n", temp ->directory);
		temp = temp->next;
	}
}

void executeAddIn(char *firstArg, char *copyArg, DirStack *stack)
{
	if (!strcmp(firstArg, "pushd")) {

		printf("firstArg is: %s\n", firstArg);
		printf("copyArg is: %s\n", copyArg);

		char *returnString = "";
		returnString = strchr(copyArg, ' ');
		if (returnString == NULL){
			printf("Error: pushd\n");
			printCompleteMessage(copyArg, 1);
		} else {
			/* Increment the pointer after strchr to reach the first character */
			if (returnString[0] == ' '){
				returnString++;
			}

			printf("returnString is: %s\n", returnString);
			printf("stack is: %s\n", stack->directory);
			//maybe remove the *
			// pushd(&stack, returnString, copyArg);
		}
	} else if (!strcmp(firstArg, "popd")) {
		// popd(&stack);
	} else {
		dirs(stack);
	}
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
	/* For cd*/
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
void executeReDirect(char *copyArg){
	return 0;
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
	DirStack *stack = NULL;
	// stack->next = NULL;

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

		/*Check for redirection */
		if (strchr(copyArg,'>')!=NULL){
			//executeReDirect(copyArg);
		/* Handles BuiltIn commands */
		}else if ((!strcmp(firstArg, "pwd")) || (!strcmp(firstArg, "cd")) || (!strcmp(firstArg, "exit"))) {
			executeBuiltIn(firstArg, copyArg);
		/* Execution for non BuiltIn commands */
		} else if ((!strcmp(firstArg, "pushd")) || (!strcmp(firstArg, "popd")) || (!strcmp(firstArg, "dirs"))) {
			executeAddIn(firstArg, copyArg, stack);
		} else { //handle all other cases
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
