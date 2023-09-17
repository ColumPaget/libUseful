#include "StreamAuth.h"
#include "OpenSSL.h"


static int STREAMAuthProcess(STREAM *S, const char *AuthTypes)
{
    char *Key=NULL, *Value=NULL, *Require=NULL;
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
								)
        {
						//does the certificate name/subject match out expectation?
						Require=OpenSSLCertDetailsGetCommonName(Require, STREAMGetValue(S, "SSL:CertificateSubject"));
						if (CompareStr(Value, Require)==0)
						{
						//is certificate valid
            if (CompareStr(STREAMGetValue(S, "SSL:CertificateVerify"), "OK")==0) AuthResult=TRUE;
						}
        }
        else if (CompareStrNoCase(Key, "issuer")==0)
        {
						//does the certificate name/subject match out expectation?
						Require=OpenSSLCertDetailsGetCommonName(Require, STREAMGetValue(S, "SSL:CertificateIssuer"));
						if (CompareStr(Value, Require)==0)
						{
						//is certificate valid
            if (CompareStr(STREAMGetValue(S, "SSL:CertificateVerify"), "OK")==0) AuthResult=TRUE;
						}
        }
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
    Destroy(Require);

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
