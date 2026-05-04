/*
Copyright (c) 2025 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef LIBUSEFUL_SECCOMP_H
#define LIBUSEFUL_SECCOMP_H



/*
This module handles setting 'seccomp rules' to be used with the seccomp security system.
Seccomp functions by blocking specified syscalls from being used.

Usually you would set this by passing the "security=" option to ProcessApplyConfig (see Process.h and
Process.c), and ProcessApplyConfig supports "Security Levels" that include seccomp settings.

'SeccompAddRules' only supports a lower-level control that allows you to specify individual syscalls.
This is specified as semicolon seperated lists of syscalls. There are four lists 'syscall_allow', which
lets syscalls run 'syscall_deny' which causes them to fail, 'syscall_kill' which immediately kills any
process that tries to use them and 'syscall_log' which logs use of syscalls in its list.

'syscall_allow' is only really used to allow running syscalls that have already been 'ruled out' by
other rules.

Some syscalls 'expand' into a list of related syscalls. For instance, specifying "chmod" will expand into "chmod;fchmod;fchmodat;lchmod". If an argument is supplied, then that is expanded too, so 'chmod(exec)' becomes "chmod(exec);fchmod(exec);fchmodat(exec);lchmod(exec)"

Expansions are:

open:     expands to "open;openat;open2;openat2;creat"
socket:   expands to "socket;socketpair;socketcall(socket);socketcall(socketpair)"
connect:  expands to "connect;socketcall(connect)"
accept:   expands to "accept;accept4;socketcall(accept);socketcall(accept4)"
bind:     expands to "bind;socketcall(bind)"
listen:   expands to "listen;socketcall(listen)"
link:     expands to "link;linkat"
symlink:  expands to "symlink;symlinkat"
unlink:   expands to "unlink;unlinkat"
mkdir:    expands to "mkdir;mkdirat"
rmdir:    expands to "rmdir;rmdirat"
rename:   expands to "rename;renameat;renameat2"
utimes:   expands to "utimes;utimesat;futimesat;utimensat;futimens"
truncate: expands to "truncate;ftruncate;truncate64;ftruncate64"
chmod:    expands to "chmod;fchmod;fchmodat;lchmod"));
chown:    expands to "chown;chown32;fchown;fchown32;fchownat;lchown;lchown32"
mmap:     expands to "mmap;mmap2"));


Some syscalls have an argument, specified as a string within brackets. If you just write the syscall
name then you're specifying a match to all calls of that syscall, but if you supply arguments in
brackets, then you are specifying a match only if the syscall is called in a way that matches that argument.

Syscalls that support arguments are:

socket:
	socket(unix)      - match 'socket' used to create UNIX local socket
	socket(local)     - match 'socket' used to create UNIX local socket
	socket(inet)      - match 'socket' used to create IPv4 socket
	socket(ip)        - match 'socket' used to create IPv4 socket
	socket(ip4)       - match 'socket' used to create IPv4 socket
	socket(inet6)     - match 'socket' used to create IPv6 socket
  socket(ip6)       - match 'socket' used to create IPv6 socket
  socket(bt)        - match bluetooth socket
  socket(bluetooth) - match bluetooth socket
  socket(alg)       - match sockets used to access kernel crypto algorithms
  socket(vsock)     - match 'vsock' sockets used to communicate between VMs
  socket(isdn)      - match ISDN subsystem sockets
  socket(can)       - match CAN networking sockets
  socket(netlink)   - match netlink sockets used to communicate with kernel subsystems

mprotect:
  mprotect(read)    - match setting memory readable
  mprotect(write)   - match setting memory writable
  mprotect(exec)    - match setting memory executable

mmap: (expands to mmap;mmap2)
   mmap(read)       - match mapping memory with read permissions
   mmap(write)      - match mapping memory with write permissions
   mmap(exec)       - match mapping memory with execute permissions

chmod: (expands to chmod;fchmod;fchmodat if an argument is given, then also expands to the 'open' group of functions that can create files with that argument)
   chmod(suid)      - match setting 'suid' bit on files
   chmod(exec)      - match setting 'exec' bit on files

open: (expands to open;openat;open2;openat2;creat)
   open(create)     - match using 'open' group syscalls to create new files
   open(write)      - match opening files for writing
   open(suid)       - match creating files with suid permissions
   open(exec)       - match creating files with exec permissions



There are also "groups" which are special names that expand to a list of syscalls. They are all
named with the prefix "group:". For example, "group:uid" expands to "setuid;setreuid;setresuid".
Available groups are:

group:create     syscalls relating to creating files: open(create);openat(create);open2(create);openat2(create);creat
group:fork       syscalls relating to process fork: clone;clone2;clone3;fork;vfork
group:uid        syscalls relating to setting process user id: setuid;setreuid;setresuid
group:ugid       syscalls relating to setting either process user or group id: setuid;setreuid;setresuid;setgid;setregid;setresgid
group:mount      syscalls relating to mount/unmount of filesystems: mount;umount;umount2
group:chroot     syscalls relating to chaning root of filesystem: chroot;pivot_root
group:kill       syscalls relating to sending signals: kill;tkill;tgkill
group:settime    syscalls relating to setting system time: settimeofday;clock_settime;clock_adjtime
group:server     syscalls relating to network servers: accept;accept4;listen and 'bind' of ports higher than 0 on x86_64
group:net        syscalls relating to both client and server networking: socket;socketcall;connect;bind;listen;accept;accept4
group:swap		   syscalls relating to system swap: swapon;swapoff
group:ns         syscalls relating to namespaces: unshare;setns
group:sysadmin   syscalls relating to system admin, including setting time, rebooting system, managing mounts, swap, etc: settimeofday;clock_settime;clock_adjtime;quotactl;reboot;swapon;swapoff;mount;umount;umount2;mknod;quotactl;sethostname;setdomainname
group:keyring    syscalls relating to keyring/credentials storage: add_key;request_key;keyctl
group:shm        syscalls relating to shared memoryshmat;shmdt;shmget;shmctl
group:fsrm       syscalls relating to deleting files and directories: unlink;rmdir
group:filesystem syscalls relating to general filesystem management: link;symlink;unlink;rmdir;rename;mkdir;mknod;chmod;chown;truncate
group:ptrace     syscalls relating to tracing or manipulating processes: ptrace;process_vm_readv;process_vm_writev;kcmp
group:kern_mod   syscalls relating to kernel modules: create_module;delete_module;init_module;finit_module;query_module
group:exec			 syscalls relating to executing programs: exec_with_loader;execv;execve;execveat
group:kexec      syscalls relating to switching kernels: kexec_load;kexec_file_load
group:shm        syscalls relating to POSIX shared memory: shmat;shmdt;shmget;shmctl
group:sem        syscalls relating to POSIX semaphores: semop;semget;semctl
group:msgq       syscalls relating to POSIX message queues: msgrcv;msgsnd;msgget;msgctl
group:ipc        syscalls relating to all POSIX interprocess communication: shmat;shmdt;shmget;shmctl;semop;semget;semctl;msgrcv;msgsnd;msgget;msgctl
group:mexec      syscalls that set memory permissions to 'exec', such syscalls are needed to load libraries and run programs



An invocation of this system would look like:

SeccompAddRules("syscall_allow=socket(unix) syscall_deny=socket;group:ptrace;chroot syscall_log=group:keyring syscall_kill=group:sysadmin")

Some care has to be taken with the order of thse rule lists. Seccomp goes with the first rule that matches.
Thus you have to first list your 'syscall_allow' rules, and then any 'syscall_kill' or 'syscall_deny' that
they overrride. In this example the 'socket' call is one that supports and argument, so we are saying
"allow the process to create local UNIX sockets, but not any other type of socket". We also deny access
to any syscalls in "group:ptrace" and specifically the "chroot" syscall.


However, all these rules can also be passed in the 'security' option of "ProcessApplyConfig" and you would
normally use that because it allows setting a wide range of options for a given process. For example:

ProcessApplyConfig("security='syscall_allow=group:keyring;group:shm untrusted+client' mdwe");

Here we specify that the process can use the system keyring and also shared memory, but is otherwise at the 'untrusted' level
of security. The final argument, 'mdwe' is an option that is outside of the 'security' setting (note the quotes around the
argument to 'security') and is not a seccomp setting, but a setting that locks memory to either be writable, or executable
but never both.
*/


#include "includes.h"


#ifdef __cplusplus
extern "C" {
#endif

int SeccompAddRules(const char *RuleList);

#ifdef __cplusplus
}
#endif



#endif
