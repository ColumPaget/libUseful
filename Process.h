/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/


#ifndef LIBUSEFUL_PROCESS_H
#define LIBUSEFUL_PROCESS_H

#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>

//Various functions related to a process

#define PROC_DAEMON            8     // make process a daemon
#define PROC_SETSID           16     // create a new session for this process
#define PROC_CTRL_TTY         32     // set stdin to be a controlling tty
#define PROC_CHROOT           64     // chroot this process before switching user etc. Used to chroot into a full unix system.
#define PROC_CONTAINER_USER  128     // unshare user and group ids
#define PROC_CONTAINER_FS    256     // unshare filesystem mounts
#define PROC_CONTAINER_UTS   512     //unshare hostname etc
#define PROC_CONTAINER_NET  1024     // unshare network namespace for this process
#define PROC_CONTAINER_PID  2048     // unshare pids namespace
#define PROC_CONTAINER_IPC  4096     // unshare ipc (shared memory, message queues, semaphors) namespace

//these must be compatible with PROC_ defines
#define SPAWN_NOSHELL        8192    // run the command directly using exec, not from a shell using system
#define SPAWN_TRUST_COMMAND 16384    // don't strip unsafe chars from command
#define SPAWN_ARG0          32768    // deduce arg[0] from the command name

#define PROC_SETUP_FAIL     65536    // internal flag if anyting goes wrong, can trigger PROC_SETUP_STRICT
#define PROC_SETUP_STRICT  131072    // if anything goes wrong in ProcessApplyConfig, then abort program
#define PROC_NO_NEW_PRIVS  262144    // do not allow privilege escalation via setuid or other such methods

#define PROC_JAIL          524288    // chroot after everything setup. This jails a process in a directory, not the same as PROC_CHROOT
#define PROC_ISOCUBE      1048576    // chroot into a tmpfs filesystem. Any files process writes will be lost when it exits
#define PROC_SIGDEF       2097152    // set default signal mask for this process
#define PROC_NEWPGROUP    4194304    // create new process group for this process
#define PROC_LAN_ONLY    33554432    // set don't route on all sockets. This is used to setup apps that we don't want to talk cross-network.
#define PROC_MDWE_HARD   67108864    // if mdwe not supported, then use seccomp to achieve similar 
#define PROC_CONTAINER_NOINIT     134217728    // within a PID container, don't launch an 'init' process (usually means only one process in this container)

#define PROC_CONTAINER (PROC_CONTAINER_FS | PROC_CONTAINER_NET | PROC_CONTAINER_PID | PROC_CONTAINER_IPC | PROC_CONTAINER_UTS | PROC_CONTAINER_USER)

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SIGNAL_HANDLER_FUNC)(int sig);
void LU_DefaultSignalHandler(int sig);

// make the current process a background process/service, with no connection to a terminal.
// Beware, this will change its pid. Returns the new pid or '0' on failure
pid_t demonize();

//try to close all file descriptors from 3 to 1024, leaving stdin, stdout and stderr alone
void CloseOpenFiles();

//write a file containing the current pid. The file can either be an absolute path to a file anywhere
//or a relative path that creates a file in /var/run. The file is locked with flock
//returns a file descriptor to the current file
int WritePidFile(const char *ProgName);

//create/use a lockfile at 'FilePath'. If file is locked already then wait 'Timeout' for it to be free
int CreateLockFile(const char *FilePath, int Timeout);

//pass argv from main to this function if you want to rewrite the process name that you see in 'ps'
void ProcessTitleCaptureBuffer(char **argv);

//set the process name that you see in 'ps'. Works like sprintf, with a format string and a variable number of
//arguments. ProcessTitleCaptureBuffer must be called first
void ProcessSetTitle(const char *FmtStr, ...);

// set 'fd' to be a processes controling tty
void ProcessSetControlTTY(int fd);

//set process to not be traceable with ptrace/strace, and also unable to make coredumps
int ProcessResistPtrace();

//set 'no new privs' to process cannot switch user/priviledges by any means (no su, sudo or setuid)
int ProcessNoNewPrivs();

//set the 'Memory Deny Write Execute' (W^X) flag. This only works in linux, and kernel 6.6 and above.
//it will tell the kernel to disallow existing non-executable mappings to be writable
//or future mappings to be writable and executable. This will prevent loading new
//libraries or execing new programs
int ProcessNoWriteExec(int Inherit);


int ProcessSecurityParseLevel(const char *Name);



/*
ProcessApplyConfig()  changes aspects of a running process. This function is not normally used in C programming, and is instead either called from the Spawn or fork functions in SpawnCommands.c or is used when binding libUseful functionality to scripting languages that have limited types, and where structures cannot easily be used to pass data.

Values that can be passed in the 'Config' string of this function are:

user=<name>        run process as user 'name'
group=<name>       run process as group 'name'
uid=<uid>          run process as user number 'uid'
gid=<gid>          run process as group number 'gid'
dir=<path>         run process in directory 'path'

setsid             start a new session for the process
newpgroup          start a new process group for the process (pgid will be the same as the processes pid)

ctrltty
ctrl_tty           set controlling tty of process to be it's standard-in. Signals and tty output will then happen on that channel, rather than via the console the program is running on. So if your overall program is running on /dev/tty1, switch it to believe it's tty is really whatever is on stdin

ctty=<fd>          set controlling tty of process to be 'fd' (where fd is a file descriptor number).
                   Same as ctrl_tty above, but using an arbitary file descriptor rather than stdin.

innull             redirect process stdin to /dev/null
outnull            redirect process stdout and stderr to /dev/null
errnull            redirect process stderr to /dev/null

stdin=<fd>         redirect process stdin to file descriptor <fd>
stdout=<fd>        redirect process stdout to file descriptor <fd>
stderr=<fd>        redirect process stderr to file descriptor <fd>

sigdef             set all signal handlers to the default values (throw away any sighandlers set by parent process)
sigdefault         set all signal handlers to the default values (throw away any sighandlers set by parent process)

demon
daemon             'daemonize' the current process (fork it into the background, close stdin/stdout/stderr, etc)

chroot=<path>      chroot process into <path>. This option happens before switching users, so that user info lookup, lockfile, pidfile, and chdir to directory specified with 'dir=<path>'  all happen within the chroot. Thus this is used when chrooting into a full unix filesystem

jail               jail the process. This does a chroot AFTER looking up the user id, creating lockfile, etc ,etc. This can be used to jail a process into an empty or private directory

strict             abort process if chdir, chroot or jail fails

namespace=<path>
ns=<path>          linux namespace to join. <path> is either a path to a namespace file, or a path to a directory (e.g. /proc/<pid>/ns )
                   that contains namespace descriptor files

nosu               set 'prctl(PR_NO_NEW_PRIVS)' to prevent privesc via su/sudo/setuid
nopriv             set 'prctl(PR_NO_NEW_PRIVS)' to prevent privesc via su/sudo/setuid
noprivs            set 'prctl(PR_NO_NEW_PRIVS)' to prevent privesc via su/sudo/setuid
nonet              run process in a network namespace container
noipc              run process in a IPC namespace container
nopid              run process in a PID namespace container
noinit             if using 'nopid' then DONT launch an init for the PID namespace container
mdwe               set 'memory deny write execute' protection for process
mdwe:inherit       set 'memory deny write execute' protection for process and inherit to child processes
mdwe:hard          set 'memory deny write execute' protection AND INHERIT to child process, but if that fails, apply seccomp rules to achive the same
m^x                set 'memory deny write execute' protection for process
m^x:inherit        set 'memory deny write execute' protection for process and inherit to child processes
m^x:hard           set 'memory deny write execute' protection AND INHERIT to child process, but if that fails, apply seccomp rules to achive the same
nice=<value>      'nice' value of new process
prio=<value>       scheduling priority of new process (equivalent to 0 - nice value)
priority=<value>   scheduling priority of new process (equivalent to 0 - nice value)
mem=<value>        resource limit for memory (data segment)
mlockmax=<value>   resource limit for locked memory
fsize=<value>      resource limit for filesize
files=<value>      resource limit for open files
coredumps=<value>  resource limit for max size of coredump files
procs=<value>      resource limit for max number of processes FOR THIS PROCESS (requires and activates pid namespace), will not apply if pid namespace can't be joined
nproc=<value>      resource limit for max number of processes FOR THIS PROCESS (requires and activates pid namespace), will not apply if pid namespace can't be joined
uprocs=<value>     resource limit for max number of processes ON A SYSTEM-WIDE PER USER BASIS.
resist_ptrace      set prctrl(PR_NONE_DUMPABLE) to prevent ptracing of the process. This also prevents coredumps totally.
openlog=<name>     set 'ident' of future syslog messages to 'name'. Also adds 'LOG_PID' and sets facility to 'LOG_USER' (see "man openlog" for more details).
mlock              lock all current and future pages in memory so they don't swap out
memlock            lock all current and future pages in memory so they don't swap out
pidfile=<path>     create pidfile for this process at 'path'
lockfile=<path>    create lockfile at 'path'

security=<level>   set security levels for seccomp. These are intended to mostly kill processess that are trying to use suspicious/dangerous or inappropriate syscalls.
   Any level includes the "nosu" setting as seccomp requires setting "prctrl(PR_NO_NEW_PRIVS)"
   Levels are 'minimal', 'basic', 'user', 'guest', 'untrusted', 'constrained', 'high', 'paranoid', 'worker' and 'memworker'. Each level includes the level below it, so 'untrusted' gives you everything in 'minimal', 'basic' and 'user'.
	 In addition to levels there are also the modifiers 'client', 'basic-net', 'local', 'nonet', 'killnet', 'nopid', 'noinit', 'noshm', 'nomsgq', 'noipc', 'noexec' and 'killexec'. These can be combined with a level using the '+' sign as in 'security=guest+local'.

  Levels:
        minimal: disable ptrace, deny ioctl(TIOCSTI) and kill apps that try to use: personality, uselib, userfaultfd, perf_event_open, kexec_load, get_kernel_syms, lookup_dcookie, vm86, vm86old, mbind, move_pages, nfsservctl, and anything involving kernel modules
        basic: everything in 'minimal' but also disable the 'acct' and 'capset' syscalls
        user: everything in 'basic' but also kill processes that try to use bpf, or any 'sysadmin' calls: settimeofday, clocksettime, clockadjtime, quotactl, reboot, swapon, swapoff, mount, umount, umount2, mknod, quotactl capset, or that try to use the TIOCSTI ioctl
        guest: everything in 'user' but also deny keyring access and kill attempts to use ptrace
        untrusted: everyting in 'user' but kill apps that try to: chroot, acct syscall, pidfd_open syscall, access the keyring, unshare or change namespaces, and deny access to utimes syscall
        constrained: everything in 'untrusted' but kill apps that try to change filesystem times, and deny all ioctls except those listed in ioctl(user)
        high: everything in 'constrained' plus deny sending signals, making filesystem links and symlinks, or chmoding files to be exectuable
        paranoid: everything in 'high' but kill processes that try to use exec to load another program, or that try to link or symlink to files, or change file timestamps, or send signals
        worker: everything in 'paranoid' plus deny making any filesystem changes. Intended for processes that just do calculations and write them to a file
        memworker: everything in 'worker' plus kill attempted use of 'open' or other filesystem calls/changes, or 'pipe'. Intended for processes that just do calculations and write them to an existing file descriptor (e.g. stdout).




  Modifiers:
        local: only allow UNIX sockets/networking. WARNING: this modifier only works on x86_64, not on x86 due to issues with socketcall syscall
        basic-net: only allow unix, inet (IPv4) and inet6 (IPv6) sockets/networking, disabling all 'weird' address families including AF_BLUETOOTH, AF_VSOCK and AF_ALG. WARNING: this modifier only works on x86_64, not on x86 due to issues with socketcall syscall
        lan: only allow connections to devices on the same ethernet segment. This doesn't use seccomp, but instead causes all sockets to be opened with the SO_DONTROUTE flag set.
        client: deny syscalls: listen and accept, preventing 'server' activity
        nonet: use namespaces to prevent network access AND/OR SECCOMP to deny networking syscalls like socket,bind and connect
        killnet: kill processes that attempt to use networking syscalls like socket,bind and connect
        netns: use namespaces to prevent access to network
        nopid: use namespaces AND/OR SECCOMP to prevent access to other processes, sending signals or even seeing what processes are running (deny 'kill' syscall)
        pidns: use namespaces to prevent access to other processes, sending signals or even seeing what processes are running
        noipc: use namespace AND/OR SECCOMP to isolate posix IPC (deny syscalls included in both 'noshm' and 'nomsgq' above along with syscalls: semop, semget and semctl)
        ipcns: use namespaces to prevent access to posix IPC (shared memory, message queues and semaphores)
        noinit: when using a PID namespace, don't create an 'init' process for it, just run the main process in the namespace
        noshm: deny 'shared memory' syscalls: shmat, shmdt, shmget and shmctl
        nomsgq: deny 'message queues' syscalls: msgrcv, msgsnd, msgget, and msgctl
        noexec: deny use of exec family of programs to load another program
        killexec: kill processes that attempt to use exec to load another program


syscall_allow=<syscall list> syscalls to allow via seccomp. This is a semi-colon seperated list of syscall names. See "Seccomp.h" for more details
syscall_deny=<syscall list> syscalls to deny via seccomp. This is a semi-colon seperated list of syscall names. See "Seccomp.h" for more details.
syscall_log=<syscall list> syscalls to log via seccomp. This is a semi-colon seperated list of syscall names. See "Seccomp.h" for more details.

All the 'syscall_' options support a few predefined 'groups' which have a prefix of 'group:' and expand into a list of syscalls. These include:

group:open       syscalls relating to opening files: open;openat;open2;openat2;creat
group:creat      syscalls relating to creating files: open(create);openat(create);open2(create);openat2(create);creat
group:fork       syscalls relating to process fork: clone;clone2;clone3;fork;vfork
group:uid        syscalls relating to setting process user id: setuid;setreuid;setresuid
group:ugid       syscalls relating to setting either process user or group id: setuid;setreuid;setresuid;setgid;setregid;setresgid
group:mount      syscalls relating to mount/unmount of filesystems: mount;umount;umount2
group:chroot     syscalls relating to chaning root of filesystem: chroot;pivot_root
group:kill       syscalls relating to sending signals: kill;tkill;tgkill
group:settime    syscalls relating to setting system time: settimeofday;clock_settime;clock_adjtime
group:server     syscalls relating to network servers: accept;accept4;listen
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
group:suid       syscalls that set the 'suid' bit on files

*/



int ProcessApplyConfig(const char *Config);

#ifdef __cplusplus
}
#endif

#endif
