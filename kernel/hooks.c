#include "hooks.h"
#include "tools.h"

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/erron.h>

// orig_clone_t orig_clone;
// asmlinkage long my_sys_clone(unsigned long flags, unsigned long newsp, int __user* parent_tidptr, int __user* child_tidptr, unsigned long tls) {
//   pr_info("filevault.hooks.my_sys_clone: sys_clone is hooked, caller: %s\n", current->comm);
//   return orig_clone(flags, newsp, parent_tidptr, child_tidptr, tls);
// }
orig_openat_t orig_openat;
asmlinkage long my_sys_openat(int dfd, const char __user *filename, int flags, umode_t mode)
{
    char fullpath[MAX_SIZE];
    get_fullpath(filename, fullpath);
    if (path_is_protected(fullpath) && !current_is_filevault())
    {
        pr_info("filevault.hooks.my_sys_openat: operation blocked .\n");
        return -EACCES;
    }

    return orig_openat(dfd, filename, flags, mode);
}

orig_unlinkat_t orig_unlinkat;
asmlinkage long my_sys_unlinkat(int dfd, const char __user *pathname, int flag)
{
    char fullpath[MAX_SIZE];
    get_fullpath(pathname, fullpath);
    if (path_is_protected(fullpath) && !current_is_filevault())
    {
        pr_info("filevault.hooks.my_sys_unlinkat: operation blocked .\n");
        return -EACCES;
    }

    return orig_unlinkat(dfd, pathname, flag);
}