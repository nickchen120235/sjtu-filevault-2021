#include <linux/module.h>
#include <linux/kernel.h>
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
#define MAX_SIZE 200
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
  copy_from_user(filevault_path, buf, count);
  len = count;
  temp = len;
  filevault_path[strcspn(filevault_path, "\r\n")] = '\0'; // remove trailing newline
  pr_info("filevault: path = %s\n", filevault_path);

  return count;
}

struct file_operations myops = {
  .read = myread,
  .write = mywrite
};
/** END OF PROC FILE */

/** SYSTEM CALL HOOKS */
// static ftrace_hook_t sys_clone_hook = HOOK("sys_clone", my_sys_clone, &orig_clone);
/** END OF SYSTEM CALL HOOKS */

static int __init filevault_init(void) {
  pr_info("filevault: filevault is inserted.\n");
  filevault_proc = proc_create("filevault_config", 0600, NULL, &myops);
  filevault_path = kmalloc(GFP_KERNEL, MAX_SIZE*sizeof(char));
  pr_info("filevault: config file is ready at /proc/filevault_config\n");

  /** INSTALL SYSTEM CALL HOOKS FROM HERE */
  // int err = install_hook(&sys_clone_hook);
  // if (err) return err;
  return 0;
}

static void __exit filevault_exit(void) {
  // remove_hook(&sys_clone_hook);
  /** REMOVE ALL SYSTEM CALL HOOKS BEFORE THIS LINE */
  proc_remove(filevault_proc);
  kfree(filevault_path);
  pr_info("filevault: filevault is removed.\n");
}

module_init(filevault_init);
module_exit(filevault_exit);
