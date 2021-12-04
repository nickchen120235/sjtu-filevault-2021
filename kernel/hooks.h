#ifndef HOOKS_H
#define HOOKS_H

#include <linux/linkage.h>

// typedef asmlinkage long (*orig_clone_t)(unsigned long, unsigned long, int __user*, int __user*, unsigned long);
// extern orig_clone_t orig_clone;
// extern asmlinkage long my_sys_clone(unsigned long flags, unsigned long newsp, int __user* parent_tidptr, int __user* child_tidptr, unsigned long tls);

typedef asmlinkage long (*orig_openat_t)(unsigned long, unsigned long, int __user *, int __user *, unsigned long);
extern orig_openat_t orig_openat;
extern asmlinkage long my_sys_openat(int dfd, const char __user *filename, int flags, umode_t mode);

typedef asmlinkage long (*orig_unlinkat_t)(unsigned long, unsigned long, int __user *, int __user *, unsigned long);
extern orig_unlinkat_t orig_unlinkat;
extern asmlinkage long my_sys_unlinkat(int dfd, const char __user *pathname, int flag);

#endif