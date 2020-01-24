# ECS-150-Project-001-
## Sample Shell

### Important Global Functions:
returnBeforeSpace: Given a string, returns everything before the first space.
Given no space, it will return the input. We do this by iterating through the
string, searching for a space ' '. When the first space is found, the original
string is copied up to that point, and the result is returned.

removeLeadingSpace: Given a string, will remove all spaces and tabs preceeding
it. We do this in a loop that gets the pointer to the string, checks the first
character to see if it is whitespace, and increments the pointer if yes, until
it hits a non-whitespace character and returns.
### Phase 1:
To run simple one word commands, we simply took the input argument, removed all
spaces following it, and then ran through a similar process as the one seen in
class. We fork a child process, get the child process to execvp the command and
exit, print a complete/error message, and return back to the shell.
### Phase 2:
To run commands with one or more arguments, we created a struct appropriately
called "structOfArgs" to store our command and all proceeding arguments. We
parse the command line input by using our returnBeforeSpace and
removeLeadingSpace functions in conjunction, which effectively returns the next
block of text that is between spaces. Each block of text is an argument, and
these arguments are stored in the structOfArgs until there entire command line
input has been parsed. At this point, we follow the same process as before,
forking a child process and then getting the child process to execvp the command
along with all arguments that are in the structOfArgs.
### Phase 3:
### Phase 4:
Since we were given that no built in commands would be redirected, we simply
created a new function that would be called upon detection of the redirection
character '>'. The function (executeRedirect) goes through a similar process
of how we regularly parse the input into our struct of arguments. The function
stops saving arguments after the character '>'. The rest of the input after the
character is then saved as a file name. We then open the file, truncating if it
exists, and creating it otherwise. A child is forked, and using dup2 we change
the output file descriptor to the given file. The child then calls Execvp on
our struct of arguments, which then executes our commands, redirecting the
output to the file. [This tutorial helped us understand dup2](https:/
        /www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/dup2.html) 
### Phase 5:
We were unfortunately unable to implement piping. We did not have enough time
and it was arguably the most complex part of the assignment.
### Phase 6:
We decided to use a linked list for the stack of directories, based on a piazza
response by Porquet.
[We consulted this website for the implementation](https://www.geeksforgeeks.org/stack-data-structure-introduction-program)
We implemented the a modified version of the link, with pushd, popd, and dirs.

Adding error redirection was as simple as creating a flag to detect an ampersand
after the file redirection character '>'. If the flag was enabled, we simply
ran a modified version of our file redirection but with the standard error
file descriptor also replaced with the given file.
