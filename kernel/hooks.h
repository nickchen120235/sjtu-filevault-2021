#ifndef HOOKS_H
#define HOOKS_H

#include <linux/linkage.h>
#include <linux/erron.h>

// typedef asmlinkage long (*orig_clone_t)(unsigned long, unsigned long, int __user*, int __user*, unsigned long);
// extern orig_clone_t orig_clone;
// extern asmlinkage long my_sys_clone(unsigned long flags, unsigned long newsp, int __user* parent_tidptr, int __user* child_tidptr, unsigned long tls);

typedef asmlinkage long (*orig_mkdir_t)(const char __user* pathname, umode_t mode);
typedef asmlinkage long (*orig_rmdir_t)(const char __user* pathname);
extern orig_mkdir_t orig_mkdir;
extern orig_rmdir_t orig_rmdir;
extern asmlinkage long my_sys_mkdir(const char __user* pathname, umode_t mode);
extern asmlinkage long my_sys_rmdir(const char __user* pathname);
#endif