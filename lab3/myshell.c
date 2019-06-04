#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#define MAX_CMDLINE_LEN 256

/* function prototypes go here... */

void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);

char* substring(const char* str, size_t begin, size_t len)
{
  if (str == 0 || strlen(str) == 0 || strlen(str) < begin || strlen(str) < (begin+len))
    return 0;

  return strndup(str + begin, len);
}

/* The main function implementation */
int main()
{
	char cmdline[MAX_CMDLINE_LEN];

	while (1)
	{
		show_prompt();
		if ( get_cmd_line(cmdline) == -1 )
			continue; /* empty line handling */

		process_cmd(cmdline);
	}
	return 0;
}


void process_cmd(char *cmdline)
{

	if (strstr(cmdline, "child ") != NULL) {
        char* substr = substring(cmdline, 6, strlen(cmdline) - 6);
        int time = atoi(substr);

        int child_status;
        pid_t child_pid;
        pid_t pid = fork();
        
        if( pid == 0 ) { // child
            printf("child pid %d is started\n", getpid());
            sleep(time);
            exit(0);
        } else {
            wait(&child_status);
            printf("child pid %d is terminated with status %d\n", pid, child_status);
        }
	} else if (strstr(cmdline, "exit") != NULL) {
            printf("myshell is terminated with pid %d\n", getpid());
        exit(0);
    }
}


void show_prompt()
{
	printf("myshell> ");
}

int get_cmd_line(char *cmdline)
{
    int i;
    int n;
    if (!fgets(cmdline, MAX_CMDLINE_LEN, stdin))
        return -1;
    // Ignore the newline character
    n = strlen(cmdline);
    cmdline[--n] = '\0';
    i = 0;
    while (i < n && cmdline[i] == ' ') {
        ++i;
    }
    if (i == n) {
        // Empty command
        return -1;
    }
    return 0;
}
