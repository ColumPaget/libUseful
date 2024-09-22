#include "Seccomp.h"

#ifdef USE_SECCOMP

#include <linux/seccomp.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/bpf.h>
#include <linux/net.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <stddef.h>  //for offsetof macro

static int InstructionCount=0;



int SysCallGetID(const char *Name)
{ 
    if (strcmp(Name, "open")==0) return(__NR_open);
    if (strcmp(Name, "openat")==0) return(__NR_openat);
    if (strcmp(Name, "unlink")==0) return(__NR_unlink);
    if (strcmp(Name, "rmdir")==0) return(__NR_rmdir);
    if (strcmp(Name, "chown")==0) return(__NR_chown);
    if (strcmp(Name, "chown32")==0) return(__NR_chown32);
    if (strcmp(Name, "fchown")==0) return(__NR_fchown);
    if (strcmp(Name, "fchownat")==0) return(__NR_fchownat);
    if (strcmp(Name, "fchown32")==0) return(__NR_fchown32);
    if (strcmp(Name, "lchown")==0) return(__NR_lchown);
    if (strcmp(Name, "lchown32")==0) return(__NR_lchown32);
    if (strcmp(Name, "ftruncate")==0) return(__NR_ftruncate);
    if (strcmp(Name, "ftruncate64")==0) return(__NR_ftruncate64);

//switch user functions
    if (strcmp(Name, "setuid")==0) return(__NR_setuid);
    if (strcmp(Name, "setreuid")==0) return(__NR_setreuid);
    if (strcmp(Name, "setresuid")==0) return(__NR_setresuid);
    if (strcmp(Name, "setgid")==0) return(__NR_setgid);
    if (strcmp(Name, "setregid")==0) return(__NR_setregid);
    if (strcmp(Name, "setresgid")==0) return(__NR_setresgid);


    if (strcmp(Name, "open")==0) return(__NR_open);


#ifdef __NR_openat
    if (strcmp(Name, "openat")==0) return(__NR_openat);
#endif

#ifdef __NR_ioctl
    if (strcmp(Name, "ioctl")==0) return(__NR_ioctl);
#endif

#ifdef __NR_bpf
    if (strcmp(Name, "bpf")==0) return(__NR_bpf);
#endif

#ifdef __NR_vm86
    if (strcmp(Name, "vm86")==0) return(__NR_vm86);
#endif

#ifdef __NR_vm86old
    if (strcmp(Name, "vm86old")==0) return(__NR_vm86old);
#endif

#ifdef __NR_move_pages
    if (strcmp(Name, "move_pages")==0) return(__NR_move_pages);
#endif

#ifdef __NR_nfsservctl
    if (strcmp(Name, "nfsservctl")==0) return(__NR_nfsservctl);
#endif

#ifdef __NR_lookup_dcookie
    if (strcmp(Name, "lookup_dcookie")==0) return(__NR_lookup_dcookie);
#endif

#ifdef __NR_execv
    if (strcmp(Name, "execv")==0) return(__NR_execv);
#endif

#ifdef __NR_execve
    if (strcmp(Name, "execve")==0) return(__NR_execve);
#endif

#ifdef __NR_execveat
    if (strcmp(Name, "execveat")==0) return(__NR_execveat);
#endif

#ifdef __NR_exec_with_loader
    if (strcmp(Name, "exec_with_loader")==0) return(__NR_exec_with_loader);
#endif

#ifdef __NR_acct
    if (strcmp(Name, "acct")==0) return(__NR_acct);
#endif
#ifdef __NR_clone
    if (strcmp(Name, "clone")==0) return(__NR_clone);
#endif
#ifdef __NR_clone2
    if (strcmp(Name, "clone2")==0) return(__NR_clone2);
#endif
#ifdef __NR_clone3
    if (strcmp(Name, "clone3")==0) return(__NR_clone3);
#endif
#ifdef __NR_fork
    if (strcmp(Name, "fork")==0) return(__NR_fork);
#endif
#ifdef __NR_vfork
    if (strcmp(Name, "vfork")==0) return(__NR_vfork);
#endif
#ifdef __NR_setsid
    if (strcmp(Name, "setsid")==0) return(__NR_setsid);
#endif
#ifdef __NR_socketcall
    if (strcmp(Name, "socketcall")==0) return(__NR_socketcall);
#endif
    if (strcmp(Name, "socket")==0) return(__NR_socket);
    if (strcmp(Name, "connect")==0) return(__NR_connect);
    if (strcmp(Name, "shutdown")==0) return(__NR_shutdown);
    if (strcmp(Name, "bind")==0) return(__NR_bind);
    if (strcmp(Name, "listen")==0) return(__NR_listen);
#ifdef __NR_accept
    if (strcmp(Name, "accept")==0) return(__NR_accept);
#endif
#ifdef __NR_accept4
    if (strcmp(Name, "accept4")==0) return(__NR_accept4);
#endif
    if (strcmp(Name, "mount")==0) return(__NR_mount);
//if (strcmp(Name, "umount")==0) return(__NR_umount);
    if (strcmp(Name, "umount2")==0) return(__NR_umount2);
    if (strcmp(Name, "unmount2")==0) return(__NR_umount2);
#ifdef __NR_create_module
    if (strcmp(Name, "create_module")==0) return(__NR_create_module);
#endif
#ifdef __NR_delete_module
    if (strcmp(Name, "delete_module")==0) return(__NR_delete_module);
#endif
#ifdef __NR_init_module
    if (strcmp(Name, "init_module")==0) return(__NR_init_module);
#endif
#ifdef __NR_finit_module
    if (strcmp(Name, "finit_module")==0) return(__NR_finit_module);
#endif
#ifdef __NR_query_module
    if (strcmp(Name, "query_module")==0) return(__NR_query_module);
#endif
#ifdef __NR_chroot
    if (strcmp(Name, "chroot")==0) return(__NR_chroot);
#endif
#ifdef __NR_pivot_root
    if (strcmp(Name, "pivot_root")==0) return(__NR_pivot_root);
#endif
#ifdef __NR_ptrace
    if (strcmp(Name, "ptrace")==0) return(__NR_ptrace);
#endif
#ifdef __NR_process_vm_readv
    if (strcmp(Name, "process_vm_readv")==0) return(__NR_process_vm_readv);
#endif
#ifdef __NR_process_vm_writev
    if (strcmp(Name, "process_vm_writev")==0) return(__NR_process_vm_writev);
#endif
#ifdef __NR_swapon
    if (strcmp(Name, "swapon")==0) return(__NR_swapon);
#endif
#ifdef __NR_swapoff
    if (strcmp(Name, "swapoff")==0) return(__NR_swapoff);
#endif
#ifdef __NR_reboot
    if (strcmp(Name, "reboot")==0) return(__NR_reboot);
#endif
#ifdef __NR_sethostname
    if (strcmp(Name, "sethostname")==0) return(__NR_sethostname);
#endif
#ifdef __NR_setdomainname
    if (strcmp(Name, "setdomainname")==0) return(__NR_setdomainname);
#endif
#ifdef __NR_unshare
    if (strcmp(Name, "unshare")==0) return(__NR_unshare);
#endif
#ifdef __NR_setns
    if (strcmp(Name, "setns")==0) return(__NR_setns);
#endif
#ifdef __NR_clock_settime
    if (strcmp(Name, "clock_settime")==0) return(__NR_clock_settime);
#endif
#ifdef __NR_clock_adjtime
    if (strcmp(Name, "clock_adjtime")==0) return(__NR_clock_adjtime);
#endif
#ifdef __NR_adjtimex
    if (strcmp(Name, "adjtimex")==0) return(__NR_adjtimex);
#endif
#ifdef __NR_settimeofday
    if (strcmp(Name, "settimeofday")==0) return(__NR_settimeofday);
#endif
#ifdef __NR_quotactl
    if (strcmp(Name, "quotactl")==0) return(__NR_quotactl);
#endif
    if (strcmp(Name, "kill")==0) return(__NR_kill);
    if (strcmp(Name, "tkill")==0) return(__NR_tkill);
    if (strcmp(Name, "tgkill")==0) return(__NR_tgkill);
    if (strcmp(Name, "utimes")==0) return(__NR_utimes);
#ifdef __NR_kexec_load
    if (strcmp(Name, "kexec_load")==0) return(__NR_kexec_load);
#endif
#ifdef __NR_kexec_file_load
    if (strcmp(Name, "kexec_file_load")==0) return(__NR_kexec_file_load);
#endif
#ifdef __NR_shmat
    if (strcmp(Name, "shmat")==0) return(__NR_shmat);
#endif
#ifdef __NR_shmdt
    if (strcmp(Name, "shmdt")==0) return(__NR_shmdt);
#endif
#ifdef __NR_shmget
    if (strcmp(Name, "shmget")==0) return(__NR_shmget);
#endif
#ifdef __NR_shmctl
    if (strcmp(Name, "shmctl")==0) return(__NR_shmctl);
#endif
#ifdef __NR_mknod
    if (strcmp(Name, "mknod")==0) return(__NR_mknod);
#endif
#ifdef __NR_mknodat
    if (strcmp(Name, "mknodat")==0) return(__NR_mknodat);
#endif

#ifdef __NR_add_key
    if (strcmp(Name, "add_key")==0) return(__NR_add_key);
#endif

#ifdef __NR_request_key
    if (strcmp(Name, "request_key")==0) return(__NR_request_key);
#endif

#ifdef __NR_keyctl
    if (strcmp(Name, "keyctl")==0) return(__NR_keyctl);
#endif

#ifdef __NR_mprotect
    if (strcmp(Name, "mprotect")==0) return(__NR_mprotect);
#endif

#ifdef __NR_uselib
    if (strcmp(Name, "uselib")==0) return(__NR_uselib);
#endif

#ifdef __NR_get_kernel_syms
    if (strcmp(Name, "get_kernel_syms")==0) return(__NR_get_kernel_syms);
#endif

#ifdef __NR_perf_event_open
    if (strcmp(Name, "perf_event_open")==0) return(__NR_perf_event_open);
#endif

#ifdef __NR_personality
    if (strcmp(Name, "personality")==0) return(__NR_personality);
#endif

#ifdef __NR_open_by_handle_at
    if (strcmp(Name, "open_by_handle_at")==0) return(__NR_open_by_handle_at);
#endif



    return(0);
}



int SocketCallGetID(const char *Name)
{
#ifdef SYS_SOCKET
if (strcmp(Name, "socket")==0) return(SYS_SOCKET);
#endif

#ifdef SYS_CONNECT
if (strcmp(Name, "connect")==0) return(SYS_CONNECT);
#endif

#ifdef SYS_BIND
if (strcmp(Name, "bind")==0) return(SYS_BIND);
#endif

#ifdef SYS_LISTEN
if (strcmp(Name, "listen")==0) return(SYS_LISTEN);
#endif

#ifdef SYS_ACCEPT
if (strcmp(Name, "accept")==0) return(SYS_ACCEPT);
#endif

#ifdef SYS_ACCEPT4
if (strcmp(Name, "accept4")==0) return(SYS_ACCEPT4);
#endif

return(-1);
}



const char *SyscallGroupLookup(const char *Name)
{
    if (strcmp(Name, "clone")==0) return("clone;clone2;clone3");
    if (strcmp(Name, "socket")==0) return("socket;socketcall(socket)");
    if (strcmp(Name, "socket(ip)")==0) return("socket(ip);socketcall(socket,ip)");
    if (strcmp(Name, "socket(unix)")==0) return("socket(unix);socketcall(socket,unix)");
    if (strcmp(Name, "connect")==0) return("connect;socketcall(connect)");
    if (strcmp(Name, "accept")==0) return("accept;accept4;socketcall(accept);socketcall(accept4)");
    if (strcmp(Name, "group:uid")==0) return("setuid;setreuid;setresuid");
    if (strcmp(Name, "group:ugid")==0) return("setuid;setreuid;setresuid;setgid;setregid;setresgid");
    if (strcmp(Name, "group:mount")==0) return("mount;umount;umount2");
    if (strcmp(Name, "group:chroot")==0) return("chroot;pivot_root");
    if (strcmp(Name, "group:kill")==0) return("kill;tkill;tgkill");
    if (strcmp(Name, "group:settime")==0) return("settimeofday;clock_settime;clock_adjtime");
    if (strcmp(Name, "group:server")==0) return("bind;accept;accept4;listen");
    if (strcmp(Name, "group:swap")==0) return("swapon;swapoff");
    if (strcmp(Name, "group:ns")==0) return("unshare;setns");
    if (strcmp(Name, "group:net")==0) return("socket;socketcall;connect;bind;listen;accept");
    if (strcmp(Name, "group:sysadmin")==0) return("settimeofday;clocksettime;clockadjtime;quotactl;reboot;swapon;swapoff;mount;umount;umount2;mknod;quotactl");
    if (strcmp(Name, "group:keyring")==0) return("add_key;request_key;keyctl");
    if (strcmp(Name, "group:shm")==0) return("shmat;shmdt;shmget;shmctl");
    if (strcmp(Name, "group:fsrm")==0) return("unlink;rmdir");
    if (strcmp(Name, "group:ptrace")==0) return("ptrace;process_vm_readv;process_vm_writev;kcmp");
    if (strcmp(Name, "group:kern_mod")==0) return("create_module;delete_module;init_module;finit_module;query_module");
    if (strcmp(Name, "group:exec")==0) return("exec_with_loader;execv;execve;execveat");
    if (strcmp(Name, "group:kexec")==0) return("kexec_load;kexec_file_load");

    return(Name);
}

int SeccompFilterAddSTMT(struct sock_filter **Filt, int Statement, uint32_t Arg)
{
    struct sock_filter stmt=BPF_STMT(Statement, Arg);

    *Filt=(struct sock_filter *) realloc(*Filt, sizeof(struct sock_filter) * (InstructionCount + 10));
    memcpy((*Filt) + InstructionCount, &stmt, sizeof(struct sock_filter));
    InstructionCount++;
    return(InstructionCount);
}

int SeccompFilterAddJUMP(struct sock_filter **Filt, int Type, int Arg, int JumpEQ, int JumpNE)
{
    struct sock_filter jump=BPF_JUMP(Type, Arg, JumpEQ, JumpNE);

    *Filt=(struct sock_filter *) realloc(*Filt, sizeof(struct sock_filter) * (InstructionCount + 10));
    memcpy((*Filt) + InstructionCount, &jump, sizeof(struct sock_filter));
    InstructionCount++;
    return(InstructionCount);
}


static int SeccompArch()
{
#ifdef __x86_64__
    return(AUDIT_ARCH_X86_64);
#endif

#ifdef _____LP64_____
    return(AUDIT_ARCH_X86_64);
#endif

    return(AUDIT_ARCH_I386);
}



void SeccompFilterAddArchCheck(struct sock_filter **Filt)
{
int expected_arch;

expected_arch=SeccompArch();

//Load the architecture of the syscall
SeccompFilterAddSTMT(Filt, BPF_LD | BPF_W | BPF_ABS, (uint32_t) (offsetof(struct seccomp_data, arch)));

//compare it with our expected arch, if it matches, all is good, and we jump over the kill instruction
SeccompFilterAddJUMP(Filt, BPF_JMP | BPF_JEQ | BPF_K, expected_arch, 1, 0);
SeccompFilterAddSTMT(Filt, BPF_RET | BPF_K, SECCOMP_RET_KILL);
}


int SeccompFilterAddSyscall(struct sock_filter **Filt, int SysCall, int Arg1, int Arg2, int Action)
{
    int jump=1;

//Load, out of the 'secomp_data' context, the value 'nr' which is the syscall number of the current syscall
    SeccompFilterAddSTMT(Filt, BPF_LD | BPF_W | BPF_ABS, (uint32_t) (offsetof(struct seccomp_data, nr)));

    if (Arg1 > -1) jump +=2;
    if (Arg2 > -1) jump +=2;
//Compare the syscall number we loaded with the one that's sypplied as 'Syscall'
//If it matches execute next instruction, otherwise jump over following instructions
    SeccompFilterAddJUMP(Filt, BPF_JMP | BPF_JEQ | BPF_K, SysCall, 0, jump);

    //if there is an argument, and it doesn't match, then again, jump one over the next instruction
    if (Arg1 > -1)
    {
				if (Arg2 > -1) jump=3;
				else jump=1;
        SeccompFilterAddSTMT(Filt, BPF_LD | BPF_W | BPF_ABS, (uint32_t) (offsetof(struct seccomp_data, args[0])));
        SeccompFilterAddJUMP(Filt, BPF_JMP | BPF_JEQ | BPF_K, Arg1, 0, jump);
    }

    if (Arg2 > -1)
    {
        SeccompFilterAddSTMT(Filt, BPF_LD | BPF_W | BPF_ABS, (uint32_t) (offsetof(struct seccomp_data, args[1])));
        SeccompFilterAddJUMP(Filt, BPF_JMP | BPF_JEQ | BPF_K, Arg2, 0, 1);
    }


		//At the moment we only allow EPERM to be returned as an errornumber, to serve seccomp_deny. 
    //The way the ERRNO is specified is a bit odd it's packed into a 32bit integer, 
    //where the top 16 bits are the actions,'SECCOMP_RET_KILL', 'SECCOMP_RET_ERRNO' etc
    //and the lower 16 bits, masked by SECCMP_RET_DATA, are the errno value to return
		if (Action==SECCOMP_RET_ERRNO) Action |= (EPERM & SECCOMP_RET_DATA);

    //kill, allow, etc
    return(SeccompFilterAddSTMT(Filt, BPF_RET | BPF_K, Action));
}


static int SeccompCommit(struct sock_filter **Filt, int Action)
{
    struct sock_fprog SeccompProg;

    SeccompFilterAddSTMT(Filt, BPF_RET | BPF_K, Action);
    SeccompProg.len = (unsigned short) InstructionCount;
    SeccompProg.filter = *Filt;

    InstructionCount=0;

    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &SeccompProg))
    {
        perror("prctl(PR_SET_SECCOMP)");
        return(FALSE);
    }

return(TRUE);
}


static int LookupSocketFamily(const char *name)
{
    	if (strcasecmp(name, "unix")==0) return(AF_UNIX);
     	if (strcasecmp(name, "ip")==0) return(AF_INET);
     	if (strcasecmp(name, "ip6")==0) return(AF_INET6);
    	if (strcasecmp(name, "bt")==0) return(AF_BLUETOOTH);
     	if (strcasecmp(name, "packet")==0) return(AF_PACKET);

return(-1);
}


void SeccompParseName(const char *Token, int *SyscallID, int *Arg1, int *Arg2)
{
    const char *p_args;
    char *Name=NULL;

    *SyscallID=-1;
		*Arg1=-1;
		*Arg2=-1;
    p_args=GetToken(Token, "(", &Name, 0);
    *SyscallID=SysCallGetID(Name);
    if (StrValid(p_args))
    {
        p_args=GetToken(p_args, ",|)", &Name, GETTOKEN_MULTI_SEP);
				if (StrValid(Name))
				{
        if (isdigit(*Name)) *Arg1=atoi(Name);
        else
        {
						switch (*SyscallID)
						{
							case __NR_socketcall:
            	*Arg1=SocketCallGetID(Name);
							break;

							case __NR_socket:
							*Arg1=LookupSocketFamily(Name);
							break;
						}
        }
        }

        p_args=GetToken(p_args, ",|)", &Name, GETTOKEN_MULTI_SEP);
				if (StrValid(Name))
				{
        if (isdigit(*Name)) *Arg1=atoi(Name);
        else
        {
						switch (*SyscallID)
						{
						case __NR_socketcall:
							//beware, socketcall has special values for it's syscall specifier
            	if (*Arg1 == SYS_SOCKET)
							{
							*Arg2=LookupSocketFamily(Name);
							}
							break;

						case __NR_mprotect:
            	if (strcasecmp(Name, "exec")==0) *Arg2=PROT_EXEC;
							break;
						}
        }
				}
    }

    Destroy(Name);
}


int SeccompFilterAddSyscallNames(struct sock_filter **Filt, const char *NameList, int Action)
{
    int syscall_id, arg1, arg2;
    int NoOfStatements=0;
    char *Token=NULL;
    const char *ptr;

    ptr=GetToken(NameList, ";", &Token, 0);
    while (ptr)
    {
				SeccompParseName(Token, &syscall_id, &arg1, &arg2);
        if (syscall_id > -1) NoOfStatements=SeccompFilterAddSyscall(Filt, syscall_id, arg1, arg2, Action);
        ptr=GetToken(ptr, ";", &Token, 0);
    }

    Destroy(Token);

    return(NoOfStatements);
}

int SeccompFilterAddSyscallGroup(struct sock_filter **Filt, const char *NameList, int Action)
{
    int NoOfStatements=0;
    char *Name=NULL;
    const char *ptr;

    ptr=GetToken(NameList, ";", &Name, 0);
    while (ptr)
    {
        NoOfStatements=SeccompFilterAddSyscallNames(Filt, SyscallGroupLookup(Name), Action);
        ptr=GetToken(ptr, ";", &Name, 0);
    }

    Destroy(Name);

    return(NoOfStatements);
}

void SeccompSetup(struct sock_filter **SeccompFilter, const char *Setup)
{
    char *Name=NULL, *Value=NULL;
    int NoOfStatements=0;
    const char *ptr;

	
    if (LibUsefulDebugActive()) fprintf(stderr, "DEBUG: Seccomp setup: %s\n", Setup);
	
    SeccompFilterAddArchCheck(SeccompFilter);
    ptr=GetNameValuePair(Setup, "\\S", "=", &Name, &Value);
    while (ptr)
    {
        if (strcmp(Name, "syscall_kill")==0) SeccompFilterAddSyscallGroup(SeccompFilter, Value, SECCOMP_RET_KILL);
        else if (strcmp(Name, "syscall_allow")==0) SeccompFilterAddSyscallGroup(SeccompFilter, Value, SECCOMP_RET_ALLOW);
        else if (strcmp(Name, "syscall_deny")==0) SeccompFilterAddSyscallGroup(SeccompFilter, Value, SECCOMP_RET_ERRNO);
        ptr=GetNameValuePair(ptr, "\\S", "=", &Name, &Value);
    }

    Destroy(Name);
    Destroy(Value);
}


int SeccompAddRules(const char *RuleList)
{
		//this will be allocated via realloc in AddStatment
    struct sock_filter *SeccompFilter=NULL;

    SeccompSetup(&SeccompFilter, RuleList);
    return(SeccompCommit( &SeccompFilter, SECCOMP_RET_ALLOW));
}

#endif

