#include "defines.h"
#include "includes.h"
#include "Vars.h"
#include "libUseful.h"
#include <sys/mman.h>

/* These functions provide an interface for setting variables that */
/* are used by libUseful itself */

int LibUsefulFlags=0;
static ListNode *LibUsefulSettings=NULL;

ListNode *LibUsefulValuesGetHead()
{
    return(LibUsefulSettings);
}

void LibUsefulInitSettings()
{
    char *Tempstr=NULL;

    LibUsefulSettings=ListCreate(LIST_FLAG_CACHE);
    SetVar(LibUsefulSettings, "LibUseful:Version", __LIBUSEFUL_VERSION__);

    Tempstr=MCopyStr(Tempstr, __LIBUSEFUL_BUILD_DATE__, " ", __LIBUSEFUL_BUILD_TIME__, NULL);
    SetVar(LibUsefulSettings, "LibUseful:BuildTime", Tempstr);

#ifdef USE_LGPL
    SetVar(LibUsefulSettings, "LibUseful:License", "LGPLv3");
#else
    SetVar(LibUsefulSettings, "LibUseful:License", "GPLv3");
#endif

    Tempstr=FormatStr(Tempstr,  "%d",  4096 * 10000);
    SetVar(LibUsefulSettings, "MaxDocumentSize",  Tempstr);

    Tempstr=FormatStr(Tempstr,  "%d",  4096 * 10000);
    SetVar(LibUsefulSettings, "WEBSOCKET:MaxFrameSize",  Tempstr);

    DestroyString(Tempstr);
}

void LibUsefulSetHTTPFlag(int Flag, const char *Value)
{
    int Flags;

    if (strtobool(Value)) Flags=HTTPGetFlags() | Flag;
    else Flags=HTTPGetFlags() & ~Flag;
    HTTPSetFlags(Flags);
}



void LibUsefulSetValue(const char *Name, const char *Value)
{
    if (! LibUsefulSettings) LibUsefulInitSettings();
    if (strcasecmp(Name,"HTTP:Debug")==0) LibUsefulSetHTTPFlag(HTTP_DEBUG, Value);
    if (strcasecmp(Name,"HTTP:NoCookies")==0) LibUsefulSetHTTPFlag(HTTP_NOCOOKIES, Value);
    if (strcasecmp(Name,"HTTP:NoCompress")==0) LibUsefulSetHTTPFlag(HTTP_NOCOMPRESS, Value);
    if (strcasecmp(Name,"HTTP:NoCompression")==0) LibUsefulSetHTTPFlag(HTTP_NOCOMPRESS, Value);
    if (strcasecmp(Name,"HTTP:NoRedirect")==0) LibUsefulSetHTTPFlag(HTTP_NOREDIRECT, Value);
    if (strcasecmp(Name,"HTTP:NoCache")==0) LibUsefulSetHTTPFlag(HTTP_NOCACHE, Value);
    if (strcasecmp(Name,"StrLenCache")==0)
    {
        if (! strtobool(Value)) LibUsefulFlags |= LU_STRLEN_NOCACHE;
        else LibUsefulFlags &= ~LU_STRLEN_NOCACHE;
    }
    SetVar(LibUsefulSettings,Name,Value);
}

const char *LibUsefulGetValue(const char *Name)
{
    if (! LibUsefulSettings) LibUsefulInitSettings();

    if (!StrValid(Name)) return("");
    return(GetVar(LibUsefulSettings,Name));
}

int LibUsefulGetBool(const char *Name)
{
    return(strtobool(LibUsefulGetValue(Name)));
}


int LibUsefulGetInteger(const char *Name)
{
    const char *ptr;

    ptr=LibUsefulGetValue(Name);
    if (StrValid(ptr)) return(atoi(ptr));
    return(0);
}

int LibUsefulDebugActive()
{
    if (StrValid(getenv("LIBUSEFUL_DEBUG"))) return(TRUE);
    if (LibUsefulGetBool("libUseful:Debug")) return(TRUE);
    return(FALSE);
}


STREAM *LibUsefulConfigFileOpen(const char *FName, const char *EnvVarName, const char *LibUsefulVar)
{
    char *Tempstr=NULL;
    STREAM *S=NULL;

    if (StrValid(LibUsefulVar)) Tempstr=CopyStr(Tempstr, LibUsefulGetValue(LibUsefulVar));
    if ( (! StrValid(Tempstr)) && (StrValid(EnvVarName)) ) Tempstr=CopyStr(Tempstr, getenv(EnvVarName));

    if (! StrValid(Tempstr)) Tempstr=MCopyStr(Tempstr, SYSCONFDIR,  "/", FName, NULL);
    if (access(Tempstr, R_OK) !=0) Tempstr=FindFileInPrefixSubDirectory(Tempstr, getenv("PATH"), "/etc/", FName);

    S=STREAMOpen(Tempstr, "r");

    Destroy(Tempstr);
    return(S);
}



void LibUsefulAtExit()
{
#ifdef HAVE_MUNLOCKALL
    if (LibUsefulFlags & LU_MLOCKALL) munlockall();
#endif

    if (LibUsefulFlags & LU_CONTAINER) FileSystemUnMount("/","lazy");
    ConnectionHopCloseAll();
    CredsStoreDestroy();
}


void LibUsefulSetupAtExit()
{
    if (! (LibUsefulFlags & LU_ATEXIT_REGISTERED)) atexit(LibUsefulAtExit);
    LibUsefulFlags |= LU_ATEXIT_REGISTERED;
}
