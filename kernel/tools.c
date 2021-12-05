/**
 * @file tools.c
 * @author 518030910031 Chen, Wei Ting
 * @brief Implementation of functions listed in tools.h
 * @date 2021-10-29
 * 
 */
#include "tools.h"

#include <linux/kernel.h>
#include <linux/version.h>

/** HOOKING */
#include <linux/kprobes.h>

static struct kprobe kp = {
  .symbol_name = "kallsyms_lookup_name",
};

/**
 * @brief Resolve the address of original syscall
 * 
 * @param hook type ftrace_hook_t* with hook->name being the function name to be searched prepared by SYSCALL_NAME(name) marco
 * @return 0 if the address is found, which in this case the address is stored in hook->address and the function is in hook->original,
 *         or non-zero if error occurs
 */
int resolve_orig_address(ftrace_hook_t* hook) {
  // find kallsyms_lookup_name
  typedef unsigned long (*kallsyms_lookup_name_t)(const char* name);
  kallsyms_lookup_name_t kallsyms_lookup_name;
  register_kprobe(&kp);
  kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
  unregister_kprobe(&kp);
  if (kallsyms_lookup_name) pr_info("filevault.tools.resolve_orig_address: kallsyms_lookup_name is found at 0x%lx\n", kallsyms_lookup_name);
  else {
    pr_err("filevault.tools.resolve_orig_address: kallsyms_lookup_name not found!!\n");
    return -1;
  }

  // find original address
  hook->address = kallsyms_lookup_name(hook->name);
  if (!hook->address) {
    pr_err("filevault.tools.resolve_orig_address: unresolved symbol: %s\n", hook->name);
    return -1;
  }
  pr_info("filevault.tools.resolve_orig_address: symbol %s found\n", hook->name);
  *((unsigned long*) hook->original) = hook->address;
  return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,11,0)

static void notrace ftrace_thunk(unsigned long ip, unsigned long parent_ip, struct ftrace_ops* ops, struct ftrace_regs* fregs) {
  ftrace_hook_t* hook = container_of(ops, ftrace_hook_t, ops);
  if (!within_module(parent_ip, THIS_MODULE)) fregs->regs.ip = (unsigned long) hook->function; // recursion prevention
}

#else

static void notrace ftrace_thunk(unsigned long ip, unsigned long parent_ip, struct ftrace_ops* ops, struct pt_regs* regs) {
  ftrace_hook_t* hook = container_of(ops, ftrace_hook_t, ops);
  if (!within_module(parent_ip, THIS_MODULE)) regs->ip = (unsigned long) hook->function; // recursion prevention
}

#endif

/**
 * @brief Hook the specified syscall
 * 
 * @param hook type ftrace_hook_t* prepared by HOOK(_name, _hook, _orig) marco
 * @return 0 if successfully hooked, otherwise non-zero
 */
int install_hook(ftrace_hook_t* hook) {
  int err;
  err = resolve_orig_address(hook);
  if (err) return err;

  hook->ops.func = ftrace_thunk;
  hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY;
  err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
  if (err) {
    pr_err("filevault.tools.install_hook: ftrace_set_filter_ip() failed: %d\n", err);
    return err;
  }

  err = register_ftrace_function(&hook->ops);
  if (err) {
    pr_err("filevault.tools.install_hook: register_ftrace_function() failed: %d\n", err);
    return err;
  }

  pr_info("filevault.tools.install_hook: function %s is hooked\n", hook->name);
  return 0;
}

/**
 * @brief Remove the hook on specified syscall
 * 
 * @param hook type ftrace_hook_t* prepared by HOOK(_name, _hook, _orig) marco
 */
void remove_hook(ftrace_hook_t* hook) {
  int err;
  err = unregister_ftrace_function(&hook->ops);
  if (err) pr_err("filevault.tools.remove_hook: unregister_ftrace_function() failed: %d\n", err);

  err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
  if (err) pr_err("filevault.tools.remove_hook: ftrace_set_filter_ip() failed: %d\n", err);

  pr_info("filevault.tools.remove_hook: function hook to %s is removed\n", hook->name);
}
/** HOOKING */

/** PATH */
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/dcache.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

/**
 * @brief Expand possibly relative path to absolute path
 * 
 * @param path pointer to an absolute or a relative path with up to ONE . OR ..
 * @param fullpath pointer to the buffer where the absolute path will be stored
 */
void get_fullpath(const char* path, char* fullpath) {
  if (path[0] == '/') {
    strncpy(fullpath, path, MAX_SIZE);
    return;
  }
  struct dentry* tmp_dentry = current->fs->pwd.dentry;
  char tmp_path[MAX_SIZE];
  char local_path[MAX_SIZE];
  memset(tmp_path, 0, MAX_SIZE);
  memset(local_path, 0, MAX_SIZE);

  // check .. case
  if (strncmp("..", path, 2) == 0) {
    tmp_dentry = tmp_dentry->d_parent;
    path = path + (strlen(path) > 2 ? 3 : 2);
  }
  // check . case
  if (strncmp(".", path, 1) == 0) path = path + (strlen(path) > 1 ? 2 : 1);

  // traverse upwards
  while (tmp_dentry != NULL) {
    if (strcmp(tmp_dentry->d_name.name, "/") == 0) break; // root folder is found
    strcpy(tmp_path, "/");
    strncat(tmp_path, tmp_dentry->d_name.name, MAX_SIZE);
    strncat(tmp_path, local_path, MAX_SIZE);
    strncpy(local_path, tmp_path, MAX_SIZE);
    tmp_dentry = tmp_dentry->d_parent;
  }
  strncpy(fullpath, local_path, MAX_SIZE);
  if (strlen(path) > 0) strncat(fullpath, "/", MAX_SIZE);
  strncat(fullpath, path, MAX_SIZE);
}

/**
 * @brief Check whether the path is protected
 * 
 * @param path pointer to an absolute or a relative path
 * @return 1 if path is protected, otherwise 0
 */
int path_is_protected(const char* path) {
  // config file case
  if (strcmp(path, "/proc/filevault_config") == 0) return 1;

  // other cases
  size_t len = strlen(filevault_path);
  if (len == 0) return 0;
  char fullpath[MAX_SIZE];
  get_fullpath(path, fullpath);
  return strncmp(fullpath, filevault_path, len) == 0;
}

char* copy_filename_from_userspace(const char __user* filename) {
  char* kernel_filename;
  kernel_filename = kmalloc(GFP_KERNEL, MAX_SIZE*sizeof(char));
  if (!kernel_filename) return NULL;

  if (copy_from_user(kernel_filename, filename, MAX_SIZE) < 0) {
    kfree(kernel_filename);
    return NULL;
  }

  return kernel_filename;
}

/** PATH */

/** PROCESS */

/**
 * @brief Check if current process or its parent is filevault
 * 
 * @return 1 if true, otherwise 0
 */
int current_is_filevault(void) {
  if (strcmp(current->comm, FILEVAULT_PROCESS) == 0) return 1;

  // look at parent if direct compare failed
  if (strcmp(current->parent->comm, FILEVAULT_PROCESS) == 0) return 1;

  // look at parent's parent
  return strcmp(current->parent->parent->comm, FILEVAULT_PROCESS) == 0;
}

/** PROCESS */
