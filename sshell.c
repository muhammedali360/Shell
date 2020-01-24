#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>

#define CMDLINE_MAX 512
#define ARGUMENTS_MAX 16
#define FAILURE 1

/* Struct to hold cmdline arguments */
typedef struct CmdLineStruct {
	char *arguments[ARGUMENTS_MAX];
} CmdLineStruct;

/* Creating built in stack */
/*Inspired by: https://www.geeksforgeeks.org/stack-data-structure-introduction-program */
typedef struct DirStack
{
	char directory[CMDLINE_MAX];
	struct DirStack* next;
} DirStack;

/* Prints complete message to stderr */
void printCompleteMessage(char *completedCommand, int exitStatus)
{
	fprintf(stderr, "+ completed '%s' [%d]\n", completedCommand, exitStatus);
}

/* Returns everything before the first whitespace */
char *returnBeforeSpace(char *string)
{
	int stringLength = strlen(string);
	char *destString = (char *)malloc(stringLength + 1);

	/* Copy everything from before the space into a new string and return said string */
	for(int i = 0; i < stringLength; i++){
		if (isspace(string[i])) {
			strncpy(destString, string, i);
			return destString;
		}
	}
	/* If the command is simply one word, free the space set aside from
	malloc because it was not used and return the orginal command */
	free(destString);
	return string;
}

/* Remove any leading spaces or tabs from the string */
char *removeLeadingSpace(char *string)
{
	/* Finds the first non white-space character and returns it. If not, keep incrementing to remove white-space characters*/
	char *firstChar = &string[0];
	for (int i = 0; i < (int)strlen(string); i++) {
		if (!isspace(*firstChar)) {
			break;
		}
		firstChar++;
	}
	return firstChar;
}

/* Create a new node and add it to the stack */
DirStack *newNode(char *directory)
{
	DirStack *node = (DirStack*)malloc(sizeof(DirStack));
	strcpy(node->directory, directory);
	node->next = NULL;
	return node;
}

int isEmpty(DirStack *root)
{
	return !root;
}

/* Pushes current directory to stack and then changes directory to given
arguement */
void pushd(DirStack **root, char *directoryToCd, char *entireCommand)
{
	char cwd[CMDLINE_MAX];
	getcwd(cwd, sizeof(cwd));
	DirStack *stackNode = newNode(cwd);
	stackNode->next = *root;
	*root = stackNode;

	int checkCd = chdir(directoryToCd);
	/* If chdir failed, then print out an error message */
	if (checkCd == -1){
		fprintf(stderr,"Error: no such directory\n");
		printCompleteMessage(entireCommand, FAILURE);
	} else {
		printCompleteMessage(entireCommand, WEXITSTATUS(EXIT_SUCCESS));
	}
}

/* Pops latest direct that was pushed onto stack, if any, and changes back to it */
void popd(DirStack **root)
{
	if (isEmpty(*root)){
		fprintf(stderr,"Error: directory stack empty\n");
		printCompleteMessage("popd", FAILURE);
		return ;
	}
	char *poppedDirectory = (char *)malloc(CMDLINE_MAX);
	DirStack *temp = *root;
	*root = (*root)->next;
	strcpy(poppedDirectory, temp->directory);
	chdir(poppedDirectory);
	free(temp);
	free(poppedDirectory);
}

void dirs(DirStack *stack)
{
	char cwd[CMDLINE_MAX];
	getcwd(cwd, sizeof(cwd));
	printf("%s\n",cwd);
	DirStack *tempStack = stack;
	while (tempStack != NULL) {
		printf("%s\n", tempStack -> directory);
		tempStack = tempStack->next;
	}
	printCompleteMessage("dirs", WEXITSTATUS(EXIT_SUCCESS));
}

/* Handles pushd, popd, and dirs */
void executeAddIn(char *firstArg, char *copyArg, DirStack **stack)
{
	if (!strcmp(firstArg, "pushd")) {
		if (!strcmp(copyArg, "pushd")){
			fprintf(stderr,"Error: no such directory\n");
			printCompleteMessage(copyArg, FAILURE);
			return ;
		}
		char *returnString = "";
		returnString = strchr(copyArg, ' ');
		if (returnString == NULL){
			fprintf(stderr,"Error: pushd\n");
			printCompleteMessage(copyArg, FAILURE);
		} else {
			/* Increment the pointer after strchr to reach the first character */
			if (returnString[0] == ' '){
				returnString++;
			}
			pushd(stack, returnString, copyArg);
		}
	} else if (!strcmp(firstArg, "popd")) {
		popd(stack);
	} else {
		dirs(*stack);
	}
}

/* Handles execution of pwd, exit and cd */
void executeBuiltIn(char *firstArg, char *entireCommand)
{
	if (!strcmp(firstArg, "pwd")) {
		char cwd[CMDLINE_MAX];
		getcwd(cwd, sizeof(cwd));
		printf("%s\n",cwd);
		printCompleteMessage(firstArg, WEXITSTATUS(0));
	} else if (!strcmp(firstArg, "exit")) {
		fprintf(stderr, "Bye...\n");
		printCompleteMessage(firstArg, WEXITSTATUS(0));
		exit(0);
	/* For cd */
	} else {
		int checkCd;
		char *returnString = "";
		returnString = strchr(entireCommand, ' ');
		/* Check if cd lacks an argument, if so print an error
		message */
		if (returnString == NULL){
			fprintf(stderr,"Error: no such directory\n");
			printCompleteMessage(entireCommand, FAILURE);
		} else {
			/* Increment the pointer after strchr to reach the first character */
			if (returnString[0] == ' '){
				returnString++;
			}
			checkCd = chdir(returnString);
			/* If checkCd failed, then print out an error message */
			if (checkCd == -1){
				fprintf(stderr,"Error: no such directory\n");
				printCompleteMessage(entireCommand, FAILURE);
			} else {
				printCompleteMessage(entireCommand, WEXITSTATUS(EXIT_SUCCESS));
			}
		}
	}
}
/* Handles redirect command if a > is found in the cmd */
void executeRedirect(char *firstArg,char *copyArg)
{
	char originalArgument[CMDLINE_MAX];
	char *restOfArg = (char *)malloc(CMDLINE_MAX);
	char *stringPtr;
	char *fileName;
	int errorFlag = 0;
	int structStart = 0;
	int fd;
	int status;
	pid_t pid;
	CmdLineStruct structOfArgs;

	strcpy(originalArgument, copyArg);
	while (1) {
		char *newArg = returnBeforeSpace(removeLeadingSpace(copyArg));
		if (strlen(newArg) == 0){
			break;
		}
		copyArg += strlen(newArg) + 1;
		structOfArgs.arguments[structStart] = (char *)malloc(CMDLINE_MAX);

		if(strchr(newArg,'>') != NULL){
			/* If the first argument is a redirect, then print
			out an error message*/
			if (!(strcmp(newArg, firstArg))) {
				fprintf(stderr, "Error: missing command\n");
				return ;
			}
			/* If everything after redirect are spaces or empty,
			then print an error message*/
			restOfArg =  strchr(originalArgument, '>');
			restOfArg++;
			if(restOfArg == strchr(originalArgument, '&')){
				/*Set error redirect flag if ampersand detected */
				errorFlag = 1;
				restOfArg++;
			}
			if  (!strlen(removeLeadingSpace(restOfArg))) {
				fprintf(stderr, "Error: no output file\n");
				return ;
			}
			fileName = returnBeforeSpace(removeLeadingSpace(restOfArg));
			fd = open(fileName, O_CREAT | O_TRUNC | O_RDWR, 0644);
			/* If fd is equal to -1, the file had an error. We
			need to either create the file or the file had an
			error. */
			if (fd == -1) {
				fprintf(stderr, "Error: cannot open output file\n");
				return ;
			}
			/* If file exists, truncate to 0 */
			lseek(fd, 0, SEEK_SET);
			stringPtr = strchr(newArg, '>');
			*stringPtr = '\0';
			if (strlen(newArg) > 0){
				strcpy(structOfArgs.arguments[structStart],  returnBeforeSpace(removeLeadingSpace(newArg)));
				structStart++;
			}
			break;
		}
		strcpy(structOfArgs.arguments[structStart], newArg);
		structStart++;
	}
	structOfArgs.arguments[structStart] = NULL;

	if (errorFlag==0){
		pid = fork();
		if (pid == 0){
			/* Child */
			dup2(fd, STDOUT_FILENO);
			close(fd);
			execvp(structOfArgs.arguments[0],
				structOfArgs.arguments);
			fprintf(stderr,"Error: command not found\n");
			exit(1);
		}  else if (pid > 0) {
			/* Parent */
			wait(&status);
			printCompleteMessage(originalArgument,
			WEXITSTATUS(status));
		} else {
			perror("fork");
			exit(1);
		}
	} else {
		/*redirect error as well */
		pid = fork();
		if (pid == 0){
			/* Child */
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			close(fd);
			execvp(structOfArgs.arguments[0],
				structOfArgs.arguments);
			fprintf(stderr,"Error: command not found\n");
			exit(1);
		}  else if (pid > 0) {
			/* Parent */
			wait(&status);
			printCompleteMessage(originalArgument,
			WEXITSTATUS(status));
		} else {
			perror("fork");
			exit(1);
		}
	}



}
int main(void)
{
	char *cmd = (char *)malloc(CMDLINE_MAX);
	char *copyArg = (char *)malloc(CMDLINE_MAX);
	char *firstArg;
	char *nl;
	CmdLineStruct structOfArgs;
	int structStart;
	DirStack **stack = (DirStack **)malloc(sizeof(DirStack **));

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
		if (structStart > ARGUMENTS_MAX){
			fprintf(stderr,"Error: too many process arguments\n");
			continue;
		}
		structOfArgs.arguments[structStart] = NULL;

		/* Check for redirection character in string  */
		if (strchr(copyArg,'>')!= NULL){
			executeRedirect(firstArg, copyArg);
		/* Handles BuiltIn commands */
		} else if ((!strcmp(firstArg, "pwd")) || (!strcmp(firstArg, "cd")) || (!strcmp(firstArg, "exit"))) {
			executeBuiltIn(firstArg, copyArg);
		/* Execution for non BuiltIn commands */
		} else if ((!strcmp(firstArg, "pushd")) || (!strcmp(firstArg, "popd")) || (!strcmp(firstArg, "dirs"))) {
			executeAddIn(firstArg, copyArg, stack);
		} else {
			/* Handle all other cases */
			pid_t pid;
			int status;

			pid = fork();
			if (pid == 0){
			/* Child */
			execvp(structOfArgs.arguments[0],
				structOfArgs.arguments);
			fprintf(stderr, "Error: command not found\n");
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
