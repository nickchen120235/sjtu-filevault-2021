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
#include <linux/string.h>
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
