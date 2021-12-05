#ifndef HOOKS_H
#define HOOKS_H

#include <linux/linkage.h>
#include <asm/ptrace.h>

typedef asmlinkage long (*orig_renameat2_t)(struct pt_regs* regs); // int olddfd, const char __user* oldname, int newdfd, const char __user* newname, unsigned int flags
extern orig_renameat2_t orig_renameat2;
extern asmlinkage long my_sys_renameat2(struct pt_regs* regs);

// sy part
typedef asmlinkage long (*orig_openat_t)(struct pt_regs* regs); // const char __user *filename, int flags, umode_t mode
extern orig_openat_t orig_openat;
extern asmlinkage long my_sys_openat(struct pt_regs* regs);

typedef asmlinkage long (*orig_unlinkat_t)(struct pt_regs* regs); // int dirfd, const char* pathname, int flags
extern orig_unlinkat_t orig_unlinkat;
extern asmlinkage long my_sys_unlinkat(struct pt_regs* regs);

// gjc part
typedef asmlinkage long (*orig_mkdir_t)(struct pt_regs* regs); // const char __user *pathname, umode_t mode
extern orig_mkdir_t orig_mkdir;
extern asmlinkage long my_sys_mkdir(struct pt_regs* regs);

typedef asmlinkage long (*orig_rmdir_t)(struct pt_regs* regs); // const char __user *pathname
extern orig_rmdir_t orig_rmdir;
extern asmlinkage long my_sys_rmdir(struct pt_regs* regs);

#endif
