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

orig_mkdir_t orig_mkdir;
asmlinkage long my_sys_mkdir(const char __user* pathname, umode_t mode) {
    char fullpath[MAX_SIZE];
    get_fullpath(pathname, fullpath);
    if (path_is_protected(fullpath) && !current_is_filevault()) {
        pr_info("filevault.hooks.my_sys_mkdir: operation blocked. \n");
        return -EACCES;
    }
    return orig_mkdir(pathname, mode);
}

orig_mkdir_t orig_rmdir;
asmlinkage long my_sys_rmdir(const char __user* pathname) {
    char fullpath[MAX_SIZE];
    get_fullpath(pathname, fullpath);
    if (path_is_protected(fullpath) && !current_is_filevault()) {
        pr_info("filevault.hooks.my_sys_rmdir: operation blocked. \n");
        return -EACCES;
    }
    return orig_rmdir(pathname);
}