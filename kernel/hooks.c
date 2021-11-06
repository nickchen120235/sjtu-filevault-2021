#include "hooks.h"

#include <linux/sched.h>
#include <linux/module.h>

// orig_clone_t orig_clone;
// asmlinkage long my_sys_clone(unsigned long flags, unsigned long newsp, int __user* parent_tidptr, int __user* child_tidptr, unsigned long tls) {
//   pr_info("filevault.hooks.my_sys_clone: sys_clone is hooked, caller: %s\n", current->comm);
//   return orig_clone(flags, newsp, parent_tidptr, child_tidptr, tls);
// }
