#include "Entropy.h"
#include "Encodings.h"
#include "Hash.h"
#include <sys/utsname.h>

int GenerateRandomBytes(char **RetBuff, int ReqLen, int Encoding)
{
    struct utsname uts;
    int i, len;
    clock_t ClocksStart, ClocksEnd;
    char *Tempstr=NULL, *RandomBytes=NULL;
    int fd;


    fd=open("/dev/urandom",O_RDONLY);
    if (fd > -1)
    {
        RandomBytes=SetStrLen(RandomBytes,ReqLen);
        len=read(fd,RandomBytes,ReqLen);
        close(fd);
    }
    else
    {
        ClocksStart=clock();
        //how many clock cycles used here will depend on overall
        //machine activity/performance/number of running processes
        for (i=0; i < 100; i++) sleep(0);
        uname(&uts);
        ClocksEnd=clock();


        Tempstr=FormatStr(Tempstr,"%lu:%lu:%lu:%lu:%llu\n",getpid(),getuid(),ClocksStart,ClocksEnd,GetTime(TIME_MILLISECS));
        //This stuff should be unique to a machine
        Tempstr=CatStr(Tempstr, uts.sysname);
        Tempstr=CatStr(Tempstr, uts.nodename);
        Tempstr=CatStr(Tempstr, uts.machine);
        Tempstr=CatStr(Tempstr, uts.release);
        Tempstr=CatStr(Tempstr, uts.version);


        len=HashBytes(&RandomBytes, "sha256", Tempstr, StrLen(Tempstr), 0);
        if (len > ReqLen) len=ReqLen;
    }


    *RetBuff=EncodeBytes(*RetBuff, RandomBytes, len, Encoding);

    DestroyString(Tempstr);
    DestroyString(RandomBytes);

    return(len);
}




char *GetRandomData(char *RetBuff, int len, char *AllowedChars)
{
    int fd;
    char *Tempstr=NULL, *RetStr=NULL;
    int i;
    uint8_t val, max_val;

    srand(time(NULL));
    max_val=StrLen(AllowedChars);

    RetStr=CopyStr(RetBuff,"");
    fd=open("/dev/urandom",O_RDONLY);
    for (i=0; i < len ; i++)
    {
        if (fd > -1) read(fd,&val,1);
        else val=rand();

        RetStr=AddCharToStr(RetStr,AllowedChars[val % max_val]);
    }

    if (fd) close(fd);

    DestroyString(Tempstr);
    return(RetStr);
}


char *GetRandomHexStr(char *RetBuff, int len)
{
    return(GetRandomData(RetBuff,len,HEX_CHARS));
}


char *GetRandomAlphabetStr(char *RetBuff, int len)
{
    return(GetRandomData(RetBuff,len,ALPHA_CHARS));
}

