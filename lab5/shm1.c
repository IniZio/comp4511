#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <sys/types.h> 

int main() {
	const int buffer_size = 1000 ;
	char *buffer; /* pointer to the shared memory */
	int segment_id;  /* unique id of the segment */

	/* Allocate a shared memory segement */
	segment_id = shmget(IPC_PRIVATE, buffer_size, IPC_CREAT | 0666);

	buffer  = (char*) shmat(segment_id, 0,0); /* attach */

	sprintf(buffer, "Modified by the parent process");

	pid_t pid = fork();
	if ( pid == 0 ) { /* Child process */
		sprintf(buffer, "Modified by Child process!");
		exit(0);
	} else { /* parent process */
		wait(0);  /* wait for the child process */
		printf("%s\n", buffer);
	}

	shmdt(buffer); 	/* detach */	
	shmctl(segment_id, IPC_RMID, 0); /* deallocate */
	return 0;
}
