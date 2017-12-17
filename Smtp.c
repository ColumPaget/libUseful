#include "Smtp.h"

int SMTPInteract(const char *Line, STREAM *S)
{
char *Tempstr=NULL;
int result=FALSE;

if (StrValid(Line)) STREAMWriteLine(Line, S);
STREAMFlush(S);
Tempstr=STREAMReadLine(Tempstr, S);

/*
syslog(LOG_DEBUG,"mail >> %s",Line);
syslog(LOG_DEBUG,"mail << %s",Tempstr);
*/

if (
		StrValid(Tempstr) && 
		( (*Tempstr=='2') || (*Tempstr=='3') )
) result=TRUE;

DestroyString(Tempstr);
return(result);
}


int SMTPHelo(STREAM *S)
{
int result;
char *Tempstr=NULL, *IP=NULL;
const char *ptr;

ptr=LibUsefulGetValue("SMTP:HELO");
if (! StrValid(ptr)) ptr=STREAMGetValue(S,"SMTP:HELO");
if (! StrValid(ptr)) 
{
	IP=GetExternalIP(IP);
	ptr=IP;
}

Tempstr=MCopyStr(Tempstr, "HELO ", ptr, NULL);
result=SMTPInteract(Tempstr, S);

DestroyString(Tempstr);
DestroyString(IP);
return(result);
}



int SendMail(const char *Sender, const char *Recipient, const char *Subject, const char *Body)
{
char *MailFrom=NULL, *Recip=NULL, *Tempstr=NULL;
char *Proto=NULL, *User=NULL, *Pass=NULL, *Host=NULL, *PortStr=NULL;
const char *p_MailServer;
int result=FALSE;
STREAM *S;

p_MailServer=LibUsefulGetValue("SMTP:Server");
if (! StrValid(p_MailServer))
{
RaiseError(0, "SendMail", "No Mailserver set");
return(FALSE);
}

if (strncmp(p_MailServer,"smtp:",5) !=0) Tempstr=MCopyStr(Tempstr,"smtp:",p_MailServer,NULL);
else Tempstr=CopyStr(Tempstr, p_MailServer);

ParseURL(Tempstr, &Proto, &Host, &PortStr, &User, &Pass, NULL, NULL);
if (! StrValid(PortStr)) PortStr=CopyStr(PortStr, "25");
Tempstr=MCopyStr(Tempstr,"tcp:",Host,":",PortStr,NULL);
//syslog(LOG_DEBUG, "mailto: %s [%s] [%s] [%s]",Tempstr,Proto,Host,PortStr);
S=STREAMOpen(Tempstr, "");
if (S)
{
	if (SMTPInteract("", S) && SMTPHelo(S))
	{
	if (SMTPInteract("STARTTLS\r\n", S)) DoSSLClientNegotiation(S, 0);
	

	MailFrom=MCopyStr(MailFrom, "MAIL FROM: ", Sender, "\r\n", NULL);
	Recip=MCopyStr(Recip, "RCPT TO: ", Recipient, "\r\n", NULL);
	if (SMTPInteract(MailFrom, S) && SMTPInteract(Recip, S) && SMTPInteract("DATA\r\n", S))
	{
		Tempstr=MCopyStr(Tempstr,"Date: ", GetDateStr("%a, %d %b %Y %H:%M:%S", NULL), "\r\n", NULL);
		Tempstr=MCatStr(Tempstr,"From: ", Recipient, "\r\n", NULL);
		Tempstr=MCatStr(Tempstr,"Subject: ", Subject, "\r\n\r\n", NULL);
		STREAMWriteLine(Tempstr, S);
		STREAMWriteLine(Body, S);
		STREAMWriteLine("\r\n.\r\n", S);
		SMTPInteract("", S);
		SMTPInteract("QUIT\r\n", S);
		result=TRUE;
	}
	else RaiseError(0,"SendMail","mailserver refused mail");
	}
	else RaiseError(0,"SendMail","Initial mailserver handshake failed");

	STREAMClose(S);
}
else RaiseError(0,"SendMail","Failed to connect to mailserver");

DestroyString(MailFrom);
DestroyString(Tempstr);
DestroyString(Recip);
DestroyString(Proto);
DestroyString(User);
DestroyString(Pass);
DestroyString(Host);
DestroyString(PortStr);

return(result);
}

