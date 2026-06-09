//Everything in here relates to cleanup operations that happen when the program shuts down

#include "defines.h"
#include "includes.h"

#include "FileSystem.h"

//for ListUsefulFlags and defines like LU_ATEXIT_REGISTERED
#include "LibSettings.h"

//for ConnectionHopCloseAll()
#include "ConnectionChain.h"

//for 'CredsStoreDestroy()'
#include "SecureMem.h"

//for munlockall
#include <sys/mman.h>


void LibUsefulAtExit()
{
#ifdef HAVE_MUNLOCKALL
    if (LibUsefulFlags & LU_MLOCKALL) munlockall();
#endif

    if (LibUsefulFlags & LU_CONTAINER_MOUNT) FileSystemUnMount("/","lazy");
    ConnectionHopCloseAll();
    CredsStoreDestroy();
}


void LibUsefulSetupAtExit()
{
    if (! (LibUsefulFlags & LU_ATEXIT_REGISTERED)) atexit(LibUsefulAtExit);
    LibUsefulFlags |= LU_ATEXIT_REGISTERED;
}





