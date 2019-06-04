#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMDLINE_LEN 256

char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if( str + len - 1 != endp )
            *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
            *str = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }


    return str;
}

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
// int main() { int pfds[2]; pipe(pfds); pid_t pid= fork(); /* 0 (child), non-zero (parent) */if( pid== 0 ) { /* The child process*/close(1); /* close stdout*/dup(pfds[1]); /* make stdoutas pipe input */close(pfds[0]); /* don't need this */execlp(p, p, NULL); } else{ /* The parent process*/close(0); /* close stdin*/dup(pfds[0]); /* make stdinas pipe output */close(pfds[1]); /* don't need this */wait(0); /* wait for the child process */execlp(p, p, "-l", NULL); } return 0; }
int main()
{
	char cmdline[MAX_CMDLINE_LEN];

	while (1)
	{
		show_prompt();
		if ( get_cmd_line(cmdline) == -1 )
			continue; /* empty line handling */

        int pos = 0;
        int infileIndex = -1;
        int hasOutfile = 0;
        char** cwds = malloc((100 + 1) * sizeof(char *));
        char pipes[100];
        for (int i = 0; i < strlen(cmdline); i++) {
            if (cmdline[i] == '<' || cmdline[i] == '|' || cmdline[i] == '>') {
                if (cmdline[i] == '<') {
                    infileIndex = pos + 1;
                }
                if (cmdline[i] == '>') {
                    hasOutfile = 1;
                }
                pipes[pos++] = cmdline[i];
            }
        }

        int isBackground = 0;
        char* infile;
        char* outfile;

        int cwdPos = -1;
        char* cwd = trim(strtok(cmdline, "<|>"));
        cwds[++cwdPos] = cwd;
        while (cwd != NULL) {
            cwd = trim(strtok(NULL, "<|>"));
            cwds[++cwdPos] = cwd;
        }

        if (infileIndex > -1) {
            strcpy(&infile, &cwds[infileIndex]);

            cwds[infileIndex] = "";
        }

        if (hasOutfile) {
            strcpy(&outfile, &cwds[cwdPos - 1]);

            cwds[cwdPos - 1] = "";
        }

        int tmpin = dup(0);
        int tmpout = dup(1);

        int fdin;
        if (strlen(infile) > 0) {
            fdin = open(infile, O_RDONLY);
        } else {
            fdin = dup(tmpin);
        }

        int ret;
        int fdout;

        for (int i = 0; i < cwdPos; i++) {
            char* action = cwds[i];
            if (strlen(action) == 0) {
                continue;
            }

            dup2(fdin, 0);
            close(fdin);

            int isLast = 1;
            for (int j = i + 1; j < cwdPos; j++) {
                if (strlen(cwds[j]) != 0) {
                    isLast = 0;
                }
            }

            if (isLast) {
                if (hasOutfile) {
                    fdout = creat(outfile, 0644);
                } else {
                    fdout = dup(tmpout);
                }
            } else {
                int fdpipe[2];
                pipe(fdpipe);
                fdout = fdpipe[1];
                fdin = fdpipe[0];
            }

            dup2(fdout, 1);
            close(fdout);

            int index = 0;
            char **args = malloc((10 + 1) * sizeof(char *));
            char* arg = strtok(action, "\t ");
            while (arg != NULL) {
                args[index++] = arg;
                arg = strtok(NULL, "\t ");
            }

            if (strcmp(args[0], "exit") == 0) {
                free(args);
                free(arg);
                exit(0);
            } else if (strcmp(args[0], "cd") == 0) {
                printf("", args[1]);
                
                // chdir(args[1] != NULL ? args[1] : getenv("HOME"));
                chdir(args[1]);
                break;
            } else if (strcmp(args[0], "child") == 0) {
                int time = atoi(args[1]);

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
                break;
            }

            ret = fork();
            if (ret == 0) {
                execvp(args[0], args);
                // _exit(0);
            }

            dup2(tmpin, 0);
            dup2(tmpout, 1);
            close(tmpin);
            close(tmpout);

            if (!isBackground) {
                waitpid(ret, NULL, 0);
                // wait(NULL);
            }
        }

        // {
        //     int index = 0;

        //     // Parse cmd into array
        //     int argcount = 100;
        //     char **args = malloc((argcount + 1) * sizeof(char *));
        //     char* arg = strtok(cwd, "\t ");
        //     while (arg != NULL) {
        //         args[index++] = arg;
        //         arg = strtok(NULL, "\t ");
        //     }

            // if (strcmp(args[0], "exit") == 0) {
            //     free(args);
            //     free(arg);
            //     exit(0);
            // } else if (strcmp(args[0], "cd") == 0) {
            //     printf("", args[1]);
                
            //     // chdir(args[1] != NULL ? args[1] : getenv("HOME"));
            //     chdir(args[1]);
            //     break;
            // } else if (strcmp(args[0], "child") == 0) {
            //     int time = atoi(args[1]);

            //     int child_status;
            //     pid_t child_pid;
            //     pid_t pid = fork();
                
            //     if( pid == 0 ) { // child
            //         printf("child pid %d is started\n", getpid());
            //         sleep(time);
            //         exit(0);
            //     } else {
            //         wait(&child_status);
            //         printf("child pid %d is terminated with status %d\n", pid, child_status);
            //     }
            //     break;
            // }

        //     if (strcmp(args[index - 1], "&") == 0) {
        //         isBackground = 1;
        //         args[index - 1] = NULL;
        //     } else {
        //         args[index] = NULL;
        //     }

        //     int inCwd = access(args[0], X_OK ) != -1;

        //     char* whichCmd[256];
        //     snprintf(whichCmd, sizeof whichCmd, "which %s > /dev/null 2>&1", args[0]);
        //     int inPATH = !system(whichCmd);

        //     if (!inCwd && !inPATH) {
        //         printf("myshell: command not found: %s\n", args[0]);
        //         break;
        //     }
        //     char* filename[256];
        //     snprintf(filename, sizeof filename, inCwd ? "./%s" : "%s", args[0]);
        //     args[0] = filename;

        //     if (fork() == 0) {
        //         execvp(filename, args);
        //     } else {
        //         if (!isBackground) {
        //             wait(NULL);
        //         }
        //     }
        // }

        // if (fork() == 0) { // child
        //     int pd[2];
        //     pipe(pd);

        //     if (fork() == 0) { // child
        //         close(1);
        //         dup2(pd[1], 1);
        //         close(pd[0]);
        //         execlp(p, p, NULL);
        //     } else {
        //         close(0);
        //         dup2(pd[0], 0);
        //         close(pd[1]);
        //         wait(0);
        //         p = strtok(NULL, " | ");
        //         execlp(p, p, NULL);
        //     }
        // } else {
        //     wait(NULL);
        // }
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
    char cwd[200];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char* segment;
        char* buffer = strtok(cwd, "/");

        while (buffer != NULL) {
            segment = buffer;
            buffer = strtok(NULL, "/");
        }
	    printf("[%s] myshell> ", strcmp(cwd, "/") == 0 ? "/" : segment);
   }
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
