#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h> 

int main() {
	const int buffer_size = 1000 ;
	int fd;

	fd = shm_open("/tmp", O_CREAT | O_RDWR , 0600) ;
	ftruncate(fd, buffer_size);

	/* Create the memory mapping */
        char *mapped_mem;
	mapped_mem = mmap(NULL, buffer_size, PROT_WRITE, MAP_SHARED, fd, 0);

	/* Write some message to the mapped memory 
	 Even the file is closed, memory update will be reflected to the shared file */
	sprintf( mapped_mem, "Write some message to the mapped memory");

	return 0;
}
