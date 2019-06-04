#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
int main() {

	const int buffer_size = 1000;
	char buffer[buffer_size] ;
	sprintf(buffer, "Modified by the parent process");

	pid_t pid = fork(); 
	if ( pid == 0 ) {  /* Child process */
		sprintf(buffer, "Modified by the child process!");
		exit(0);
	} else { /* parent process */
		wait(0);  /* wait for the child process */
		printf("%s\n", buffer);
	}
	return 0;
}
