#include "StreamAuth.h"
#include "OpenSSL.h"


//did the client provide an SSL certificate as authentication?
static int STREAMAuthProcessCertificate(STREAM *S, const char *CertName, const char *CommonName)
{
char *Require=NULL;
int AuthResult=FALSE;

//does the certificate name/subject match out expectation?
Require=OpenSSLCertDetailsGetCommonName(Require, STREAMGetValue(S, CommonName));
if (CompareStr(CertName, Require)==0)
{
		//is certificate valid
    if (CompareStr(STREAMGetValue(S, "SSL:CertificateVerify"), "OK")==0) AuthResult=TRUE;
}

Destroy(Require);
return(AuthResult);
}


static int STREAMAuthProcess(STREAM *S, const char *AuthTypes)
{
    char *Key=NULL, *Value=NULL;
    const char *ptr;
    int AuthResult=FALSE;

    ptr=GetNameValuePair(AuthTypes, ",", ":",&Key, &Value);
    while (ptr)
    {
        if (CompareStrNoCase(Key, "basic")==0)
        {
            if (CompareStr(Value, STREAMGetValue(S, "Auth:Basic"))==0) AuthResult=TRUE;
        }
        else if (
									(CompareStrNoCase(Key, "certificate")==0) ||
									(CompareStrNoCase(Key, "cert")==0)
								)  AuthResult=STREAMAuthProcessCertificate(S, Value, "SSL:CertificateSubject");
        else if (CompareStrNoCase(Key, "issuer")==0) AuthResult=STREAMAuthProcessCertificate(S, Value, "SSL:CertificateIssuer");
        else if (strncasecmp(Key, "cookie:", 7)==0)
        {
            if (CompareStr(Value, STREAMGetValue(S, Key))==0) AuthResult=TRUE;
        }
        else if (CompareStrNoCase(Key, "ip")==0)
        {
            if (CompareStr(Value, GetRemoteIP(S->in_fd))==0) AuthResult=TRUE;
        }
        ptr=GetNameValuePair(ptr, ",", "=",&Key, &Value);
    }

    Destroy(Key);
    Destroy(Value);

    return(AuthResult);
}



int STREAMAuth(STREAM *S)
{
const char *ptr;

ptr=STREAMGetValue(S, "Authentication");
if (! StrValid(ptr)) return(TRUE);

return(STREAMAuthProcess(S, ptr));

return(FALSE);
}
