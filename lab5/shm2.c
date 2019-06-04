#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h> 

int main() {
	const int buffer_size = 1000 ;
	char buffer[buffer_size]; /* pointer to the shared memory */
	int i, fd; /* file descriptor  */

	for (i=0;i<buffer_size;i++)
		buffer[i]=' ';
	buffer[buffer_size-1]='\0';	

	/* Prepare a file with the size of buffer_size */
	fd = open("./tmp", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR) ;
	write(fd, buffer, buffer_size);

	/* Create the memory mapping */
        char *mapped_mem;
	mapped_mem = mmap(NULL, buffer_size, PROT_WRITE, MAP_SHARED, fd, 0);

	close(fd); /* close the file */

	/* Write some message to the mapped memory 
	 Even the file is closed, memory update will be reflected to the shared file */
	sprintf( mapped_mem, "Write some message to the mapped memory");

	munmap(mapped_mem, buffer_size); /* Release the mapped memory*/
	return 0;
}
