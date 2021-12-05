#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#include "tools.h"
#include "hooks.h"

MODULE_LICENSE("GPL");

/** GLOBAL VARIABLES */
char* filevault_path;
/** END OF GLOBAL VARIABLES */

/** PROC FILE */
char* msg;
int len, temp;
static struct proc_dir_entry* filevault_proc;

static ssize_t myread(struct file* f, char __user* buf, size_t count, loff_t* pos) {
  if (count > temp) count = temp;
  temp = temp - count;
  copy_to_user(buf, filevault_path, count);
  if (count == 0) temp = len;
  return count;
}

static ssize_t mywrite(struct file* f, const char __user* buf, size_t count, loff_t* pos) {
  copy_from_user(msg, buf, count);
  len = count;
  temp = len;
  msg[strcspn(msg, "\r\n")] = '\0'; // remove trailing newline
  memset(filevault_path, 0, MAX_SIZE);
  get_fullpath(msg, filevault_path);
  len = strlen(filevault_path);
  pr_info("filevault: path = %s\n", filevault_path);

  return count;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)

static const struct proc_ops myops = {
  .proc_read = myread,
  .proc_write = mywrite
};

#else

static const struct file_operations myops = {
  .read = myread,
  .write = mywrite
};

#endif
/** END OF PROC FILE */

/** SYSTEM CALL HOOKS */
static ftrace_hook_t sys_openat_hook = HOOK("sys_openat", my_sys_openat, &orig_openat);
static ftrace_hook_t sys_unlinkat_hook = HOOK("sys_unlinkat", my_sys_unlinkat, &orig_unlinkat);
static ftrace_hook_t sys_mkdir_hook = HOOK("sys_mkdir", my_sys_mkdir, &orig_mkdir);
static ftrace_hook_t sys_rmdir_hook = HOOK("sys_rmdir", my_sys_rmdir, &orig_rmdir);
static ftrace_hook_t sys_renameat2_hook = HOOK("sys_renameat2", my_sys_renameat2, &orig_renameat2);
/** END OF SYSTEM CALL HOOKS */

static int __init filevault_init(void) {
  pr_info("filevault: filevault is inserted.\n");
  filevault_proc = proc_create("filevault_config", 0600, NULL, &myops);
  filevault_path = kmalloc(GFP_KERNEL, MAX_SIZE*sizeof(char));
  msg = kmalloc(GFP_KERNEL, MAX_SIZE*sizeof(char));
  pr_info("filevault: config file is ready at /proc/filevault_config\n");

  /** INSTALL SYSTEM CALL HOOKS FROM HERE */
  int err = install_hook(&sys_openat_hook);
  if (err) return err;
  err = install_hook(&sys_unlinkat_hook);
  if (err) return err;
  err = install_hook(&sys_mkdir_hook);
  if (err) return err;
  err = install_hook(&sys_rmdir_hook);
  if (err) return err;
  err = install_hook(&sys_renameat2_hook);
  if (err) return err;
  return 0;
}

static void __exit filevault_exit(void) {
  remove_hook(&sys_openat_hook);
  remove_hook(&sys_unlinkat_hook);
  remove_hook(&sys_mkdir_hook);
  remove_hook(&sys_rmdir_hook);
  remove_hook(&sys_renameat2_hook);
  /** REMOVE ALL SYSTEM CALL HOOKS BEFORE THIS LINE */
  proc_remove(filevault_proc);
  kfree(filevault_path);
  kfree(msg);
  pr_info("filevault: filevault is removed.\n");
}

module_init(filevault_init);
module_exit(filevault_exit);
