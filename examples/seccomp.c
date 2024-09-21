#include "../libUseful.h"
#include <sys/ptrace.h>

main()
{
pid_t pid;

ProcessApplyConfig("security=constrained");
pid=fork();
if (pid == 0)
{
//ProcessApplyConfig("security=user");

//chroot(".");
printf("After chroot\n");

ptrace(PTRACE_CONT, getppid(), 0, 0);
printf("After ptrace\n");
exit(1);
}
waitpid(pid, 0, 0);

}
