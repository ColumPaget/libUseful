#include "SysInfo.h"
#include "Socket.h" // for MAX_HOST_NAME

#include <sys/utsname.h>

#ifdef linux
#include <sys/sysinfo.h>
#endif

#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>


char *OSSysInfoInterfaces(char *RetStr)
{
    struct if_nameindex *interfaces;
    int i;

    RetStr=CopyStr(RetStr, "");
    interfaces=if_nameindex();
    if (interfaces)
    {
        for (i=0; interfaces[i].if_name != NULL; i++)
        {
            RetStr=MCatStr(RetStr, interfaces[i].if_name, " ", NULL);
        }
        if_freenameindex(interfaces);
    }

    return(RetStr);
}


char *OSSysInfoLocaleConf(char *RetStr, int Type)
{
    struct lconv *Details;

    RetStr=CopyStr(RetStr, "");
    setlocale(LC_ALL, "");
    Details=localeconv();
    if (Details)
    {
        switch (Type)
        {
        case OSINFO_CURRENCY:
            RetStr=CopyStr(RetStr, Details->int_curr_symbol);
            break;
        case OSINFO_CURRENCY_SYM:
            RetStr=CopyStr(RetStr, Details->currency_symbol);
            break;
        }
    }

    return(RetStr);
}


//This is a convinience function for use by modern languages like
//lua that have an 'os' object that returns information
const char *OSSysInfoString(int Info)
{
    static struct utsname UtsInfo;
    struct passwd *pw;
    const char *ptr;
    static char *buf=NULL;
    int result;

    uname(&UtsInfo);

    switch (Info)
    {
    case OSINFO_TYPE:
        return(UtsInfo.sysname);
        break;

    case OSINFO_ARCH:
        return(UtsInfo.machine);
        break;

    case OSINFO_RELEASE:
        return(UtsInfo.release);
        break;

    case OSINFO_HOSTNAME:
        buf=SetStrLen(buf, HOST_NAME_MAX);
        result=gethostname(buf, HOST_NAME_MAX);
        //if call failed make sure we return a blank string
        if (result != 0) buf[0]='\0';
        return(buf);
        break;

    case OSINFO_DOMAINNAME:
        buf=SetStrLen(buf, HOST_NAME_MAX);
        result=getdomainname(buf, HOST_NAME_MAX);
        //if call failed make sure we return a blank string
        if (result != 0) buf[0]='\0';
        return(buf);
        break;

    case OSINFO_HOMEDIR:
        pw=getpwuid(getuid());
        if (pw) return(pw->pw_dir);
        break;

    case OSINFO_TMPDIR:
        ptr=getenv("TMPDIR");
        if (! ptr) ptr=getenv("TEMP");
        if (! ptr) ptr="/tmp";
        if (ptr) return(ptr);
        break;

    case OSINFO_LOCALE:
        return(getenv("LANG"));
        break;

    case OSINFO_LANG:
        buf=CopyStr(buf, getenv("LANG"));
        StrTruncChar(buf, '_');
        return(buf);
        break;

    case OSINFO_COUNTRY:
        buf=CopyStr(buf, getenv("LANG"));
        StrTruncChar(buf, '.');
        ptr=strchr(buf, '_');
        if (ptr) return(ptr+1);
        return(buf);
        break;

    case OSINFO_CURRENCY:
    case OSINFO_CURRENCY_SYM:
        buf=OSSysInfoLocaleConf(buf, Info);
        return(buf);
        break;

    case OSINFO_INTERFACES:
        //don't just return output of function, as buf is static we must update it
        buf=OSSysInfoInterfaces(buf);
        return(buf);
        break;


        /*
        case OSINFO_USERINFO:
          pw=getpwuid(getuid());
          if (pw)
          {
            MuJSNewObject(TYPE_OBJECT);
            MuJSNumberProperty("uid",pw->pw_uid);
            MuJSNumberProperty("gid",pw->pw_gid);
            MuJSStringProperty("username",pw->pw_name);
            MuJSStringProperty("shell",pw->pw_shell);
            MuJSStringProperty("homedir",pw->pw_dir);
          }
        break;
        }
        */

    }


    return("");
}


//This is a convienice function for use by modern languages like
//lua that have an 'os' object that returns information
size_t OSSysInfoLong(int Info)
{
    int result;
    double loadavg[3];

#ifdef HAVE_SYSINFO
    struct sysinfo SysInfo;

    sysinfo(&SysInfo);
    switch (Info)
    {
    case OSINFO_UPTIME:
        return((size_t) SysInfo.uptime);
        break;

    case OSINFO_TOTALMEM:
        return((size_t) (SysInfo.totalram * SysInfo.mem_unit));
        break;

    case OSINFO_FREEMEM:
        return((size_t) (SysInfo.freeram * SysInfo.mem_unit));
        break;

    case OSINFO_BUFFERMEM:
        return((size_t) (SysInfo.bufferram * SysInfo.mem_unit));
        break;

    case OSINFO_TOTALSWAP:
        return((size_t) (SysInfo.totalswap * SysInfo.mem_unit));
        break;

    case OSINFO_FREESWAP:
        return((size_t) (SysInfo.freeswap * SysInfo.mem_unit));
        break;

    case OSINFO_PROCS:
        return((size_t) SysInfo.procs);
        break;

    case OSINFO_LOAD1MIN:
        return((size_t) SysInfo.loads[0]);
        break;

    case OSINFO_LOAD5MIN:
        return((size_t) SysInfo.loads[1]);
        break;

    case OSINFO_LOAD15MIN:
        return((size_t) SysInfo.loads[2]);
        break;
    }

#endif


    switch (Info)
    {
#ifdef HAVE_GETLOADAVG

    case OSINFO_LOAD1MIN:
        result=getloadavg(loadavg, 3);
        if (result > -1) return((size_t) loadavg[0]);
        break;

    case OSINFO_LOAD5MIN:
        result=getloadavg(loadavg, 3);
        if (result > -1) return((size_t) loadavg[1]);
        break;

    case OSINFO_LOAD15MIN:
        result=getloadavg(loadavg, 3);
        if (result > -1) return((size_t) loadavg[2]);
        break;

#endif

    case OSINFO_PAGESIZE:
#ifdef HAVE_GETPAGESIZE
        return((size_t) getpagesize());
#endif

#ifdef HAVE_SYSCONF
        return((size_t) sysconf(_SC_PAGESIZE));
#endif

#ifdef PAGE_SIZE
        return((size_t) PAGE_SIZE);
#endif
        break;

#ifdef HAVE_SYSCONF
    case OSINFO_OPENMAX:
        return((size_t) sysconf(_SC_OPEN_MAX));
        break;
    case OSINFO_CLOCKTICK:
        return((size_t) sysconf(_SC_CLK_TCK));
        break;
#endif
    }


    return(0);
}
