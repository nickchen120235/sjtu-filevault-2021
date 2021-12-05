#include "hooks.h"
#include "tools.h"

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

orig_renameat2_t orig_renameat2;
asmlinkage long my_sys_renameat2(struct pt_regs* regs) {
  char src_fullpath[MAX_SIZE];
  char dest_fullpath[MAX_SIZE];
  char* src_kfilename = copy_filename_from_userspace((char*)regs->si);
  char* dest_kfilename = copy_filename_from_userspace((char*)regs->cx);
  get_fullpath(src_kfilename, src_fullpath);
  get_fullpath(dest_kfilename, dest_fullpath);
  if ((path_is_protected(src_fullpath) || path_is_protected(dest_fullpath)) && !current_is_filevault()) {
    pr_info("filevault.hooks.my_sys_renameat2: opeation blocked.\n");
    kfree(src_kfilename);
    kfree(dest_kfilename);
    return -EACCES;
  }

  kfree(src_kfilename);
  kfree(dest_kfilename);
  return orig_renameat2(regs);
}

// sy part
orig_openat_t orig_openat;
asmlinkage long my_sys_openat(struct pt_regs* regs)
{
  char fullpath[MAX_SIZE];
  char* kfilename = copy_filename_from_userspace((char*)regs->si);
  get_fullpath(kfilename, fullpath);
  if (path_is_protected(fullpath) && !current_is_filevault())
  {
    pr_info("filevault.hooks.my_sys_openat: operation blocked.\n");
    kfree(kfilename);
    return -EACCES;
  }

  kfree(kfilename);
  return orig_openat(regs);
}

orig_unlinkat_t orig_unlinkat;
asmlinkage long my_sys_unlinkat(struct pt_regs* regs)
{
  char fullpath[MAX_SIZE];
  char* kpathname = copy_filename_from_userspace((char*)regs->si);
  get_fullpath(kpathname, fullpath);
  if (path_is_protected(fullpath) && !current_is_filevault())
  {
    pr_info("filevault.hooks.my_sys_unlinkat: operation blocked.\n");
    kfree(kpathname);
    return -EACCES;
  }

  kfree(kpathname);
  return orig_unlinkat(regs);
}

// gjc part
orig_mkdir_t orig_mkdir;
asmlinkage long my_sys_mkdir(struct pt_regs* regs)
{
  char fullpath[MAX_SIZE];
  char* kpathname = copy_filename_from_userspace((char*)regs->di);
  get_fullpath(kpathname, fullpath);
  if (path_is_protected(fullpath) && !current_is_filevault())
  {
    pr_info("filevault.hooks.my_sys_mkdir: operation blocked. \n");
    kfree(kpathname);
    return -EACCES;
  }

  kfree(kpathname);
  return orig_mkdir(regs);
}

orig_rmdir_t orig_rmdir;
asmlinkage long my_sys_rmdir(struct pt_regs* regs)
{
  char fullpath[MAX_SIZE];
  char* kpathname = copy_filename_from_userspace((char*)regs->di);
  get_fullpath(kpathname, fullpath);
  if (path_is_protected(fullpath) && !current_is_filevault())
  {
    pr_info("filevault.hooks.my_sys_rmdir: operation blocked. \n");
    kfree(kpathname);
    return -EACCES;
  }

  kfree(kpathname);
  return orig_rmdir(regs);
}
