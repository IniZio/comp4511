/* test_xmerge.c–a user-space test program, it can be placed anywhere */
/* include necessary user-space header files */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>


#define SYSCALL_NUM_XMERGE 356

/* Note: Except from __user flag, xmerge_param is equivalent to the struct we defined in the kernel space */

struct xmerge_param {
  char* outfile;
  char** infiles;
  unsigned int num_files;
  int oflags;
  mode_t mode;
  int* ofile_count;
};

int main(int argc, const char*argv[]) {
  long res;
  struct xmerge_param ps;

  /* add other local variables here */
  /* initialize local variables and provide default values */
  int opt; // argument options
  int append = 0;
  int createIfMissing = 0;

  ps.mode = S_IRUSR | S_IWUSR;

  /* handle input arguments */
  while((opt = getopt(argc, argv, "acm:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'a':  
              append = O_APPEND;
              // printf("Append mode: %d\n", append);
              break;
            case 'c':
              createIfMissing = O_CREAT;
              // printf("Create mode: %d\n", createIfMissing);
              break;
            case 'm':
              ps.mode = atoi(optarg);
              // printf("permissions: %d\n", ps.mode);
        }  
    }

    ps.oflags = append | createIfMissing | O_WRONLY;
    // printf("oflags: %d\n", ps.oflags);
      
    ps.infiles = malloc((100) * sizeof(char *));
    int index = -1;
    ps.outfile = argv[optind++];
    // printf("outfile: %s\n", ps.outfile);  

    for(; optind < argc; optind++){
        ps.infiles[index++] = argv[optind];
        // printf("infiles: %s\n", ps.infiles[index-1]);  
    }
    ps.num_files = index + 1;
    // printf("number of input files: %d\n", ps.num_files);

    ps.ofile_count = 0;

  /* Here, we invoke syscall using the POSIX API */
  res = syscall(SYSCALL_NUM_XMERGE, &ps, sizeof(struct xmerge_param));

  /* Output of the test program */
  if (res >= 0) {
    printf("In total, %d files have been successfully merged!\n", *(ps.ofile_count));
    printf("The total read size: %d bytes.\n", res);
  } else {
    /* Show appropriate error messages here */
    printf("error message? %i\n", res);
  }
  
  /* Remember to do any memory cleanup in user space, if needed */
  
  return 0;
}
