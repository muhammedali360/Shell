#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define ARGUMENTS_MAX 16
#define TOKEN_MAX 32

int main(int argc, char *argv[])
{
        char cmd[CMDLINE_MAX];
        pid_t pid;
        int status;

        while (1) {
                char *nl;
                // int retval;

                char *nullTest[1] = {NULL};
                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

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
                        break;
                }

                /* Regular command */
                // retval = system(cmd);
                // fprintf(stdout, "Return status value for '%s': %d\n",
                //         cmd, retval);
                if (argc == 1) {
                  pid = fork();
                  if (pid == 0) {
                    execvp(argv[1],nullTest);
                    perror("execvp");
                    exit(1);
                  } else if (pid > 0) {
                    waitpid(pid, &status, 0);
                    printf("Child exited with status: %d\n", WEXITSTATUS(status));
                  } else {
                    perror("fork");
                    exit(1);
                  }
                }
                // retval = system(cmd);
                fprintf(stderr, "+ completed '%s' [%d]\n", argv[1], status);
        }

        return EXIT_SUCCESS;
}
