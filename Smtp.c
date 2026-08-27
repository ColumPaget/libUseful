#include "Smtp.h"
#include "Encodings.h"
#include "URL.h"
#include "Inet.h"

#define SMTP_CAP_HELO 1
#define SMTP_CAP_EHLO 2
#define SMTP_CAP_AUTH_LOGIN 4
#define SMTP_CAP_AUTH_PLAIN 8

char *SMTPRead(char *RetStr, STREAM *S)
{
    char *Tempstr=NULL;

    RetStr=CopyStr(RetStr, "");
#ifdef USE_SMTP
    Tempstr=STREAMReadLine(Tempstr, S);
    while (StrLen(Tempstr) > 3)
    {
        RetStr=CatStr(RetStr,Tempstr);
        if (Tempstr[3] == ' ') break;
        Tempstr=STREAMReadLine(Tempstr, S);
    }

    if (LibUsefulDebugActive()) fprintf(stderr, "SMTP: << %s\n", Tempstr);

    DestroyString(Tempstr);
#endif

    return(RetStr);
}


int SMTPWrite(const char *Line, STREAM *S)
{
#ifdef USE_SMTP
    if (StrValid(Line))
    {
        STREAMWriteLine(Line, S);
        if (LibUsefulDebugActive()) fprintf(stderr, "SMTP: >> %s\n", Line);

        STREAMFlush(S);
        return(TRUE);
    }
#endif

    return(FALSE);
}


#ifdef USE_SMTP //all the below functions will be compiled out if 'configure --disable-smtp' is selected

static int SMTPInteract(const char *Line, STREAM *S)
{
    char *Tempstr=NULL;
    int result=FALSE;


    SMTPWrite(Line, S);
    Tempstr=SMTPRead(Tempstr, S);


    /*
    syslog(LOG_DEBUG,"mail >> %s",Line);
    syslog(LOG_DEBUG,"mail << %s",Tempstr);
    */

    if (
        StrValid(Tempstr) &&
        ( (*Tempstr=='2') || (*Tempstr=='3') )
    ) result=atoi(Tempstr);
    else result=FALSE;

    DestroyString(Tempstr);
    return(result);
}


static int SMTPParseCapabilities(const char *String)
{
    char *Token=NULL;
    const char *ptr;
    int Caps=0;

    ptr=GetToken(String+4," ",&Token,0);
    while (ptr)
    {
        if (strcasecmp(Token, "LOGIN")==0) Caps |= SMTP_CAP_AUTH_LOGIN;
        if (strcasecmp(Token, "PLAIN")==0) Caps |= SMTP_CAP_AUTH_PLAIN;
        ptr=GetToken(ptr," ",&Token,0);
    }

    DestroyString(Token);

    return(Caps);
}


static int SMTPHelo(STREAM *S)
{
    int RetVal=0;
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

    ptr=LibUsefulGetValue("SMTP:HELO");
    if (! StrValid(ptr)) ptr=STREAMGetValue(S,"SMTP:HELO");
    if (! StrValid(ptr))
    {
        Token=GetExternalIP(Token);
        LibUsefulSetValue("SMTP:HELO", Token);
        ptr=Token;
    }

    Tempstr=MCopyStr(Tempstr, "EHLO ", ptr, "\r\n", NULL);
    SMTPWrite(Tempstr,  S);
    Tempstr=SMTPRead(Tempstr, S);

    if (StrValid(Tempstr) && (*Tempstr=='2'))
    {
        RetVal |= SMTP_CAP_EHLO;
        ptr=GetToken(Tempstr,"\n",&Token,0);
        while (ptr)
        {
            StripTrailingWhitespace(Token);
            RetVal |= SMTPParseCapabilities(Token);
            ptr=GetToken(ptr,"\n",&Token,0);
        }
    }
//Some old server that doesn't support EHLO, switch to HELO
    else
    {
        Tempstr=MCopyStr(Tempstr, "HELO ", ptr, "\r\n", NULL);
        if (SMTPInteract(Tempstr, S)) RetVal |= SMTP_CAP_HELO;
    }

    DestroyString(Tempstr);
    DestroyString(Token);
    return(RetVal);
}



static int SMTPLogin(STREAM *S, int Caps, const char *User, const char *Pass)
{
    char *Tempstr=NULL, *Base64=NULL, *ptr;
    int len, result, RetVal=FALSE;


    if (Caps & SMTP_CAP_AUTH_LOGIN)
    {
        Tempstr=CopyStr(Tempstr, "AUTH LOGIN\r\n");
        result=SMTPInteract(Tempstr, S);

        //if mailserver tells us '235' straight away, then it's telling
        //us we don't need to authenticate
        if (result == 235) RetVal=TRUE;
        else if (result == 334)
        {
            Base64=EncodeBytes(Base64, User, StrLen(User), ENCODE_BASE64);
            Tempstr=MCopyStr(Tempstr, Base64, "\r\n", NULL);
            if (SMTPInteract(Tempstr, S))
            {
                Base64=EncodeBytes(Base64, Pass, StrLen(Pass), ENCODE_BASE64);
                Tempstr=MCopyStr(Tempstr, Base64, "\r\n", NULL);
                if (SMTPInteract(Tempstr, S)) RetVal=TRUE;
            }
        }
        else RaiseError(0, "SMTPLogin", "SMTP Login Conversation Failed");
    }
    else if (Caps & SMTP_CAP_AUTH_PLAIN)
    {
        Tempstr=SetStrLen(Tempstr, StrLen(User) + StrLen(Pass) +10);

        //this isn't what it looks like. The '\0' here do not terminate the string
        //as this authentication system uses a string with '\0' as separators
        len=StrLen(User);
        ptr=Tempstr;
        memcpy(ptr, User, len);
        ptr+=len;
        *ptr='\0';
        ptr++;

        len=StrLen(Pass);
        memcpy(ptr, Pass, len);
        ptr+=len;
        *ptr='\0';
        ptr++;

        Base64=EncodeBytes(Base64, Tempstr, ptr-Tempstr, ENCODE_BASE64);
        Tempstr=MCopyStr(Tempstr, "AUTH PLAIN ", Base64, "\r\n",NULL);
        if (SMTPInteract(Tempstr, S)) RetVal=TRUE;
    }

    DestroyString(Tempstr);
    DestroyString(Base64);

    return(RetVal);
}


static int SmtpSendRecipients(const char *Recipients, STREAM *S)
{
    char *Recip=NULL, *Tempstr=NULL;
    const char *ptr;
    int result=FALSE;

    ptr=GetToken(Recipients, ",", &Recip, GETTOKEN_HONOR_QUOTES);
    while (ptr)
    {
        Tempstr=MCopyStr(Tempstr, "RCPT TO: ", Recip, "\r\n", NULL);
        if (SMTPInteract(Tempstr, S)) result=TRUE;
        ptr=GetToken(ptr, ",", &Recip, GETTOKEN_HONOR_QUOTES);
    }

    DestroyString(Tempstr);
    DestroyString(Recip);
    return(result);
}



static char *SMTPRationalizeProtocol(char *Proto, int Flags)
{
    if (Flags & LU_SMTP_NOSSL) Proto=CopyStr(Proto, "tcp");
    else if (Flags & LU_SMTP_INITIAL_SSL) Proto=CopyStr(Proto, "tls");
    else if (Flags & LU_SMTP_STARTTLS) Proto=CopyStr(Proto, "tcp");

    if (StrValid(Proto))
    {
        if (strcasecmp(Proto, "smtp")==0) Proto=CopyStr(Proto, "tcp");
        else if (strcasecmp(Proto, "smtps")==0) Proto=CopyStr(Proto, "tls");
        else if (strcasecmp(Proto, "tcp")==0) Proto=CopyStr(Proto, "tcp");
        else if (strcasecmp(Proto, "tls")==0) Proto=CopyStr(Proto, "tls");
        else if (strcasecmp(Proto, "ssl")==0) Proto=CopyStr(Proto, "tls");
        else Proto=CopyStr(Proto, "tcp");
    }

    return(Proto);
}

#endif //all the above functions are compiled out if 'configure --disable-smtp' is selected

STREAM *SMTPConnectServer(const char *ServerURL, const char *Sender, const char *Recipients, int Flags)
{
    STREAM *S=NULL;

#ifdef USE_SMTP
    char *Recip=NULL, *Tempstr=NULL;
    char *Proto=NULL, *User=NULL, *Pass=NULL, *Host=NULL, *PortStr=NULL;
    int result=FALSE, Caps=0;

    if (! StrValid(ServerURL))
    {
        RaiseError(0, "SendMail", "No Mailserver set");
        return(NULL);
    }


    ParseURL(ServerURL, &Proto, &Host, &PortStr, &User, &Pass, NULL, NULL);
    Proto=SMTPRationalizeProtocol(Proto, Flags);
    if (! StrValid(PortStr)) PortStr=CopyStr(PortStr, "25");

    //if we are connecting with TLS from the get-go, then we dont' want to do
    //STARTTLS below
    if (strcmp(Proto, "tls")==0) Flags |= LU_SMTP_NOSSL;


    Tempstr=MCopyStr(Tempstr, Proto, ":", Host, ":", PortStr, NULL);


//syslog(LOG_DEBUG, "mailto: %s [%s] [%s] [%s]",Tempstr,Proto,Host,PortStr);

    S=STREAMOpen(Tempstr, "");
    if (S)
    {
        if (SMTPInteract("", S))
        {
            Caps=SMTPHelo(S);

            if (Caps > 0)
            {
                //try STARTTLS, the worst that will happen is the server will say no
                if ((! (Flags & LU_SMTP_NOSSL)) && SSLAvailable() && SMTPInteract("STARTTLS\r\n", S))
                {
                    DoSSLClientNegotiation(S, 0);
                    Caps=SMTPHelo(S);
                }

                if (
                    (Caps & (SMTP_CAP_AUTH_LOGIN | SMTP_CAP_AUTH_PLAIN)) &&
                    (StrValid(User) && StrValid(Pass))
                ) SMTPLogin(S, Caps, User, Pass);

                //Whether login was needed or not,  worked or not, let's try to send a mail
                Tempstr=MCopyStr(Tempstr, "MAIL FROM: ", Sender, "\r\n", NULL);
                if (! SMTPInteract(Tempstr, S)) RaiseError(0,"SendMail","mailserver refused sender");
                else if (! SmtpSendRecipients(Recipients, S)) RaiseError(0,"SendMail","No recipients accepted by mailserver");
                else if (! SMTPInteract("DATA\r\n", S)) RaiseError(0,"SendMail","mailserver refused mail");
                else
                {
                    //we got this far, rest of the process is handled by the calling function
                    result=TRUE;
                }
            }
            else RaiseError(0,"SendMail","Initial mailserver handshake failed");
        }
        else RaiseError(0,"SendMail","Initial mailserver handshake failed");
    }
    else RaiseError(0,"SendMail","mailserver connection failed");


    DestroyString(Tempstr);
    DestroyString(Recip);
    DestroyString(Proto);
    DestroyString(User);
    DestroyString(Pass);
    DestroyString(Host);
    DestroyString(PortStr);

    if (! result)
    {
        STREAMClose(S);
        return(NULL);
    }

#endif

    return(S);
}


STREAM *SMTPConnect(const char *Sender, const char *Recipients, int Flags)
{
    return(SMTPConnectServer(LibUsefulGetValue("SMTP:Server"), Sender, Recipients, Flags));
}


int SMTPSendMailViaServer(const char *ServerURL, const char *Sender, const char *Recipient, const char *Subject, const char *Body, int Flags)
{
    int result=FALSE;

#ifdef USE_SMTP
    char *Tempstr=NULL;
    STREAM *S;

    S=SMTPConnectServer(ServerURL, Sender, Recipient, Flags);
    if (S)
    {
        if (! (Flags & LU_SMTP_NOHEADER))
        {
            Tempstr=MCopyStr(Tempstr,"Date: ", GetDateStr("%a, %d %b %Y %H:%M:%S", NULL), "\r\n", NULL);
            Tempstr=MCatStr(Tempstr,"From: ", Sender, "\r\n", NULL);
            Tempstr=MCatStr(Tempstr,"To: ", Recipient, "\r\n", NULL);
            Tempstr=MCatStr(Tempstr,"Subject: ", Subject, "\r\n\r\n", NULL);
            STREAMWriteLine(Tempstr, S);
        }
        STREAMWriteLine(Body, S);
        STREAMWriteLine("\r\n.\r\n", S);
        SMTPInteract("", S);
        SMTPInteract("QUIT\r\n", S);
        result=TRUE;

        STREAMClose(S);
    }

    DestroyString(Tempstr);
#endif

    return(result);
}

int SMTPSendMail(const char *Sender, const char *Recipients, const char *Subject, const char *Body, int Flags)
{
    return(SMTPSendMailViaServer(LibUsefulGetValue("SMTP:Server"), Sender, Recipients, Subject, Body, Flags));
}


int SMTPSendMailFileViaServer(const char *ServerURL, const char *Sender, const char *Recipients, const char *Path, int Flags)
{
    int result=FALSE;

#ifdef USE_SMTP
    char *Tempstr=NULL;
    STREAM *S, *F;

    F=STREAMOpen(Path, "r");
    if (F)
    {
        S=SMTPConnectServer(ServerURL, Sender, Recipients, Flags);
        if (S)
        {
            STREAMSendFile(F, S, 0, SENDFILE_LOOP);
            STREAMWriteLine("\r\n.\r\n", S);
            SMTPInteract("", S);
            SMTPInteract("QUIT\r\n", S);
            result=TRUE;

            STREAMClose(S);
        }
        STREAMClose(F);
    }
    else RaiseError(0,"SMTPSendMailFile","Failed to open file for sending");

    DestroyString(Tempstr);
#endif

    return(result);
}


int SMTPSendMailFile(const char *Sender, const char *Recipients, const char *Path, int Flags)
{
    return(SMTPSendMailFileViaServer(LibUsefulGetValue("SMTP:Server"), Sender, Recipients, Path, Flags));
}


