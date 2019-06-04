#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/processor.h>
#include <linux/slab.h>

struct xmerge_param {
  __user char *outfile;
  __user char **infiles;
  unsigned int num_files;

  int oflags;
  mode_t mode;
  __user int *ofile_count;
};

SYSCALL_DEFINE2(xmerge, void*, args, size_t, argslen) {
  struct xmerge_param ps;

  if (copy_from_user(&ps, args, argslen) != 0) {
    return -EFAULT;
  };

  /* Implement error handling cases in appropriate places below */
  /* Local variable of the xmerge syscall */
  int outfile;

  /* Hint: You can use printk to help you debug your program */

  /* Before the loop,add these 2 lines for kernel file I/O operations */
  mm_segment_t old_fs;
  old_fs = get_fs();

  outfile = ksys_open(ps.outfile, ps.oflags, ps.mode);

  if (outfile < 0) {
    return outfile;
  }

  int total = 0;
  int BUFFER_SIZE = 100;
  char *buffer
  int last = BUFFER_SIZE;
  int infileIndex = 0;

  /* Write a loop to merge each input files */
  for (; infileIndex < ps.num_files; infileIndex++) {
    int infile = ksys_open(ps.infiles[infileIndex], 0, O_RDONLY);

    if (infile < 0) {
      return infile;
    }

    while(last >= BUFFER_SIZE) {
      buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
      int result = ksys_read(infile, buffer, BUFFER_SIZE);

      if(result < 0) return result;

      total += result;
      last = result;
        
      result = ksys_write(outfile, buffer, BUFFER_SIZE);

      if (result < 0) return result;
      kfree(buffer);
    }
    ksys_close(infile);
    ps.ofile_count++;
  }
  ksys_close(outfile);

  /* After all file operations, restore the old_fs */
  set_fs(get_ds());

  set_fs(old_fs);

  /* Remember to do any memory cleanup in kernel space, if needed */
  /* Return the number of bytes successfully merged */

  if (copy_to_user(&ps, args, argslen) != 0) {
    return -EFAULT;
  };

  return total;
}
