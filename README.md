# Kernel Module Developer Documentation

## I. Environment Setup
1. Download Ubuntu Server 20.04 from [here](https://ubuntu.com/download/server)
2. Install on whatever virtual machine
3. Update and upgrade packages (`sudo` as required)
    ```
    apt update && apt upgrade -y
    ```
4. Install the following packages and reboot
    ```
    build-essential linux-generic-hwe-20.04
    ```
5. Check current kernel version (`uname -r`), it should be
    ```
    5.11.0-xx-generic
    ```
6. Done

> Note: Lower kernel version may work but tested only on `5.11.0-38 generic`

## II. Helper Functions
Defined in `kernel/tools.h`

### *macro* `HOOK(_name, _hook, _orig)`
Expands to the first three members in a `ftrace_hook` structure
- `_name`: name of syscall to be hooked
- `_hook`: pointer to the modified syscall function
    > Note: The function should behave as the original syscall or call the original syscall at the end
-  `_orig`: pointer to an object that looks like the original function
    > Note: Since the "real" original function will be determined during `install_hook`, what is needed is really a variable which shares the same signature as the original function
    >
    > ex. Consider in `hooks.h`
    > ```c
    > typedef asmlinkage long (*orig_clone_t)(unsigned long, unsigned long, int __user*, int __user*, unsigned long);
    > orig_clone_t orig_clone;
    > ```
    > Then `orig_clone` is what to put in this field

Usage: if one wants hook "sys_clone", then **in `main.c`** he should include
```c
static ftrace_hook_t sys_clone_hook = HOOK("sys_clone", my_sys_clone, &orig_clone)
```
with `my_sys_clone` **defined in `hooks.h`**

### *struct* `ftrace_hook`, *typedef* `ftrace_hook_t`
Defines the structure to store hooking information
- `name`: type `const char*`, original function name
- `function`: type `void*`, pointer to function hook
- `original`: type `void*`, pointer to the original function
    > Note: The three items above **should be prepared using `HOOK` macro**, see the above section for example
- `address`: type `unsigned long`, address to the original function
- `ops`: type `struct ftrace_ops`, for ftrace

### *function* `int install_hook(ftrace_hook_t* hook)`
Hook the specified system call

Usage
```c
/** somewhere in the code */
static ftrace_hook_t sys_clone_hook = HOOK("sys_clone", my_sys_clone, &orig_clone)

/** inside init function */
err = install_hook(&my_sys_clone);
if (err) return err;
```

#### parameters
- `hook`, type `ftrace_hook_t*`, struct to function to be hooked

#### return
`0` if successfully hooked, non-zero otherwise

### *function* `void remove_hook(ftrace_hook_t* hook)`
Remove the hook on specified function

Usage
```c
/** inside exit function */
remove_hook(&my_sys_clone);
```

#### parameters
- `hook`, type `ftrace_hook_t*`, struct to function whose hook to be removed

### *function* `void get_fullpath(const char* path, char* fullpath)`
Expand possibly relative path to absolute path

Usage
```c
// inside /home/nick with subfolder coding
char fullpath[MAX_SIZE];
get_fullpath("/home/nick/coding", fullpath); // "/home/nick/coding"
get_fullpath("./coding", fullpath);          // "/home/nick/coding"
get_fullpath(".", fullpath);                 // "/home/nick"
get_fullpath("..", fullpath);                // "/home"
```

#### parameter
- `path`, type `const char*`, pointer to an absolute or a relative path **with up to ONE `.` or `..`**
- `fullpath`, type `char*`, pointer to the buffer where the absolute path will be stored

### *function* `int path_is_protected(const char* path)`
Check whether the path is protected, by implementation the config file `/proc/filevault_config` is considered protected

Usage
```c
// assume /home/nick is protected, currently inside /home/nick
path_is_protected("/proc/filevault_config"); // 1, config file is protected
path_is_protected("/home/nick");             // 1, the protected path
path_is_protected("/home/nick/coding");      // 1, subfolders are protected
path_is_protected("/home");                  // 0, parent folder is not protected
path_is_protected("./coding");               // 1, subfolder case with relative path
path_is_protected("..");                     // 0, parent folder case with relative path
```

#### parameter
- `path`, type `const char*`, pointer to an absolute or a relative path **with up to ONE `.` or `..`**

#### return
`1` if `path` is protected, otherwise `0`

### *function* `int current_is_filevault(void)`
Check if current process or its parent is `filevault`

#### return
`1` if true, otherwise `0`

## III. Where to put code
**Kernel module: `kernel/`**
- helper functions: `extern` definition in `tools.h`, implementation in `tools.c`
- hook functions:

  - `typedef` original function signature in `hooks.h`
  - `extern` original function object in `hooks.h`, don't forget to actually define it in `hooks.c`
  - `extern` hook function definition in `hooks.h`, implementation in `hooks.c`
> See commented code in `hooks.h` and `hooks.c` for example

## IV. Notes
1. **Create a branch for yourself! Don't touch the `master` branch**
2. Open a [pull request](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request) to let me know your code is ready for merging
3. Should you have any question, find me wherever you can or open an issue