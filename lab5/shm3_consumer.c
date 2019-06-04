#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h> 

int main() {
	const int buffer_size = 1000 ;
	int fd;

	fd = shm_open("/tmp", O_RDONLY , 0600) ;

	/* memory mapping */
        char *mapped_mem;
	mapped_mem = mmap(NULL, buffer_size, PROT_READ, MAP_SHARED, fd, 0);
	if ( mapped_mem == MAP_FAILED) {
		printf("Map failed\n");
		exit(-1);
	}

	/* Write the message */
	printf( "%s\n", mapped_mem );

	if ( shm_unlink("/tmp") == -1 ) {
		printf("Error removing /tmp");
		exit(-1);
	}
	return 0;
}
