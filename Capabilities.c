#include "Capabilities.h"
#include "Process.h"

#ifdef USE_CAPABILITIES
#ifdef HAVE_LIBCAP

#include <sys/capability.h>
#include <linux/securebits.h>
#include <sys/prctl.h>



static char *ProcessBuildCapabilitiesString(char *RetStr, const char *CapNames, int Inherit)
{
    char *Token=NULL;
    const char *ptr;

    ptr=GetToken(CapNames, ",",&Token,0);
    while (ptr)
    {
        StrTrim(Token);
        if (Inherit) RetStr=MCatStr(RetStr, Token, "=pei ", NULL);
        else RetStr=MCatStr(RetStr, Token, "=pe ", NULL);
        ptr=GetToken(ptr, ",",&Token,0);
    }

    Destroy(Token);

    return(RetStr);
}
#endif
#endif



int ProcessSetCapabilities(const char *Capabilites, const char *InheritCapabilities, int Flags)
{
    char *CapsStr=NULL, *Tempstr=NULL;
//default is to indicate failure.
//if no Capabilities or InheritCapabilities asked for we will return 0 immediately
//else we will only return 0 on success
    int RetVal=PROC_SETUP_FAIL;

//we want to return FALSE straight away to prevent 'Capabilities not compiled in' debug warnings
    if ( (! StrValid(Capabilites)) && (! StrValid(InheritCapabilities)) ) return(0);

#ifdef USE_CAPABILITIES
#ifdef HAVE_LIBCAP

    cap_t caps;


    if (Flags & LU_CAPABILITIES_UID) CapsStr=CopyStr(CapsStr, "cap_setuid=pe cap_setgid=pe ");
    CapsStr=ProcessBuildCapabilitiesString(CapsStr, Capabilites, FALSE);
    CapsStr=ProcessBuildCapabilitiesString(CapsStr, InheritCapabilities, TRUE);

    if (Flags & LU_CAPABILITIES_KEEP)
    {
        prctl(PR_SET_SECUREBITS, SECBIT_KEEP_CAPS);
        prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
    }

    if (LibUsefulDebugActive()) fprintf(stderr, "DEBUG: LinuxCapabilities: %s  keepcaps=%d\n", CapsStr, Flags && LU_CAPABILITIES_KEEP);

    caps=cap_from_text(CapsStr);
    if (caps)
    {
        if (cap_set_proc(caps) == 0)
        {
            //set 'RetVal' to 0 to indicate everything worked
            RetVal=0;
        }
        cap_free(caps);

    }
    else RaiseError(ERRFLAG_ERRNO, "capabilities", "bad capabilities string: %s", CapsStr);


#else
    RaiseError(0, "capabilities", "linux capabilites not compiled in - libcap not found at compile time");
#endif

#else
    RaiseError(0, "capabilities", "linux capabilites not compiled in - not selected at compile time");
#endif

    Destroy(Tempstr);
    Destroy(CapsStr);
    return(RetVal);
}

