/**
 * @file tools.h
 * @author 518030910031 Chen, Wei Ting
 * @brief Helper function header file
 * @date 2021-10-29
 * 
 */

#ifndef HOOK_TOOLS_H
#define HOOK_TOOLS_H
/**
 * Hooking
 */
#include <linux/ftrace.h>

/**
 * use the following marco to prepare a hook
 * 
 * SYSCALL_NAME(name) expands to a proper x64 syscall entry listed in /proc/kallsyms
 * For example, SYSCALL_NAME(sys_clone) expands to "__x64_sys_clone"
 * 
 * HOOK(_name, _hook, _orig) expands to the first three members in a ftrace_hook structure
 * _name: name of syscall to be hooked
 *        notice that _name will be expanded using SYSCALL_NAME(_name), 
 *        so just put in the function name (e.g. sys_openat) and it will work
 * _hook: pointer to the modified syscall function
 *        notice that the function should behave as the original syscall or call the original syscall at the end
 * _orig: pointer to an object that looks like the original function
 *        this is a tricky one. since the "real" original function will be determined during install_hook,
 *        what is needed is really a variable which shares the same signature as the original function
 *        ex. consider
 *        typedef asmlinkage long (*orig_clone_t)(unsigned long, unsigned long, int __user*, int __user*, unsigned long);
 *        orig_clone_t orig_clone;
 *        then "orig_clone" is what to put in this field
 * For example, if one wants to hook "sys_clone", then the code will look like
 * static ftrace_hook_t sys_clone_hook = HOOK("sys_clone", my_sys_clone, &orig_clone)
 * with my_sys_clone being the self-defined sys_clone function
 * notice the "&" before orig_clone (which is defined as a VARIABLE, not POINTER)
 * 
 */
#define SYSCALL_NAME(name) ("__x64_" name)
#define HOOK(_name, _hook, _orig) { \
  .name     = SYSCALL_NAME(_name),  \
  .function = (_hook),              \
  .original = (_orig)               \
}

/**
 * @brief struct to store hooking information
 * 
 */
typedef struct ftrace_hook {
  const char* name;      // original function name
  void* function;        // function hook
  void* original;        // original function
  unsigned long address; // address to original function
  struct ftrace_ops ops; // for ftrace
} ftrace_hook_t;

extern int resolve_orig_address(ftrace_hook_t* hook);
extern int install_hook(ftrace_hook_t* hook);
extern void remove_hook(ftrace_hook_t* hook);

#endif
