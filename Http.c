#include "Http.h"
#include "HttpChunkedTransfer.h"
#include "DataProcessing.h"
#include "ConnectionChain.h"
#include "ContentType.h"
#include "Hash.h"
#include "URL.h"
#include "OAuth.h"
#include "Time.h"
#include "base64.h"
#include "SecureMem.h"
#include "Errors.h"
#include "Entropy.h"
#include "HttpUtil.h"

/* These functions relate to CLIENT SIDE http/https */


#define HTTP_OKAY 0
#define HTTP_NOCONNECT 1
#define HTTP_NOTFOUND 2
#define HTTP_REDIRECT 3
#define HTTP_ERROR 4
#define HTTP_CIRCULAR_REDIRECTS 5
#define HTTP_NOTMODIFIED 6


#define HTTP_HEADERS_SENT 1
#define HTTP_CLIENTDATA_SENT 2
#define HTTP_HEADERS_READ 4



const char *HTTP_AUTH_BY_TOKEN="AuthTokenType";
static ListNode *Cookies=NULL;
static int g_HTTPFlags=0;



void HTTPInfoDestroy(void *p_Info)
{
    HTTPInfoStruct *Info;

    if (! p_Info) return;
    Info=(HTTPInfoStruct *) p_Info;
    DestroyString(Info->Protocol);
    DestroyString(Info->Method);
    DestroyString(Info->Host);
    DestroyString(Info->Doc);
    DestroyString(Info->UserName);
    DestroyString(Info->UserAgent);
    DestroyString(Info->Destination);
    DestroyString(Info->ResponseCode);
    DestroyString(Info->PreviousRedirect);
    DestroyString(Info->RedirectPath);
    DestroyString(Info->ContentType);
    DestroyString(Info->Timestamp);
    DestroyString(Info->PostData);
    DestroyString(Info->PostContentType);
    DestroyString(Info->Proxy);
    DestroyString(Info->Credentials);
    DestroyString(Info->Authorization);
    DestroyString(Info->ProxyAuthorization);
    DestroyString(Info->ConnectionChain);

    ListDestroy(Info->ServerHeaders,Destroy);
    ListDestroy(Info->CustomSendHeaders,Destroy);
    free(Info);
}





static void HTTPSetLoginCreds(HTTPInfoStruct *Info, const char *User, const char *Password)
{
    if (Info->AuthFlags == 0) Info->AuthFlags |= HTTP_AUTH_BASIC;
    if (Info->AuthFlags != HTTP_AUTH_OAUTH)
    {
        Info->UserName=CopyStr(Info->UserName, User);

//if (StrValid(Password)) CredsStoreAdd(Info->Host, Info->UserName, Password);
        Info->Credentials=CopyStr(Info->Credentials, Password);
    }
}


void HTTPInfoSetValues(HTTPInfoStruct *Info, const char *Host, int Port, const char *Logon, const char *Password, const char *Method, const char *Doc, const char *ContentType, int ContentLength)
{
    Info->State=0;
    Info->PostData=CopyStr(Info->PostData,"");
    Info->Host=CopyStr(Info->Host,Host);
    if (Port > 0) Info->Port=Port;
    else Info->Port=0;
    Info->Method=CopyStr(Info->Method,Method);
    Info->Doc=CopyStr(Info->Doc,Doc);
    Info->PostContentType=CopyStr(Info->PostContentType,ContentType);
    Info->PostContentLength=ContentLength;

    if (StrValid(Logon) || StrValid(Password)) HTTPSetLoginCreds(Info, Logon, Password);
}




HTTPInfoStruct *HTTPInfoCreate(const char *Protocol, const char *Host, int Port, const char *Logon, const char *Password, const char *Method, const char *Doc, const char *ContentType, int ContentLength)
{
    HTTPInfoStruct *Info;
    const char *ptr;

    Info=(HTTPInfoStruct *) calloc(1, sizeof(HTTPInfoStruct));
    Info->Protocol=CopyStr(Info->Protocol,Protocol);
    HTTPInfoSetValues(Info, Host, Port, Logon, Password, Method, Doc, ContentType, ContentLength);

    Info->ServerHeaders=ListCreate();
    Info->CustomSendHeaders=ListCreate();
    SetVar(Info->CustomSendHeaders,"Accept","*/*");

    if (g_HTTPFlags) Info->Flags=g_HTTPFlags;

    ptr=LibUsefulGetValue("HTTP:Proxy");
    if (StrValid(ptr))
    {
        Info->Proxy=CopyStr(Info->Proxy,ptr);
        strlwr(Info->Proxy);
        if (strncmp(Info->Proxy,"http:",5)==0) Info->Flags |= HTTP_PROXY;
        else if (strncmp(Info->Proxy,"https:",6)==0) Info->Flags |= HTTP_PROXY;
        else Info->Flags=HTTP_TUNNEL;
    }

    Info->UserAgent=CopyStr(Info->UserAgent, LibUsefulGetValue("HTTP:UserAgent"));

    return(Info);
}


char *HTTPInfoToURL(char *RetStr, HTTPInfoStruct *Info)
{
    char *p_proto;
    char *Doc=NULL;

    if (Info->Flags & HTTP_SSL) p_proto="https";
    else p_proto="http";

    Doc=HTTPQuoteChars(Doc,Info->Doc," ");
    RetStr=FormatStr(RetStr, "%s://%s:%d%s",p_proto,Info->Host,Info->Port,Info->Doc);

    DestroyString(Doc);
    return(RetStr);
}


void HTTPInfoPOSTSetContent(HTTPInfoStruct *Info, const char *ContentType, const char *ContentData, int ContentLength, int Flags)
{
//if there were arguments in URL, and HTTP_POSTARGS is set, then post these
//otherwise include them in the URL again
    if (Flags & HTTP_POSTARGS)
    {
        if (StrValid(ContentData))
        {
            if (ContentLength == 0) ContentLength=StrLen(ContentData);

            Info->PostData=SetStrLen(Info->PostData,ContentLength);
            memcpy(Info->PostData, ContentData, ContentLength);
            Info->PostContentLength=StrLen(Info->PostData);
        }

        if (StrValid(ContentType)) Info->PostContentType=CopyStr(Info->PostContentType,ContentType);
        else if (ContentLength > 0) Info->PostContentType=CopyStr(Info->PostContentType,"application/x-www-form-urlencoded; charset=UTF-8");

        if (ContentLength) Info->PostContentLength=ContentLength;
    }
    else if (StrValid(ContentData))
    {
        Info->Doc=MCatStr(Info->Doc,"?",ContentData, NULL);
        Info->PostData=CopyStr(Info->PostData, "");
    }
}

void HTTPInfoSetURL(HTTPInfoStruct *Info, const char *Method, const char *iURL)
{
    char *URL=NULL, *Proto=NULL, *User=NULL, *Pass=NULL, *Token=NULL, *Args=NULL, *Value=NULL;
    const char *p_URL, *ptr;

    ptr=GetToken(iURL, "\\S", &URL, 0);
    p_URL=strrchr(URL,'|');
    if (p_URL)
    {
        Info->ConnectionChain=CopyStrLen(Info->ConnectionChain, URL, p_URL - URL);
        p_URL++;
    }
    else p_URL=URL;

    if (strcasecmp(Method,"POST")==0) ParseURL(p_URL, &Proto, &Info->Host, &Token, &User, &Pass, &Info->Doc, &Args);
    else ParseURL(p_URL, &Proto, &Info->Host, &Token, &User, &Pass, &Info->Doc, NULL);

    if (! StrValid(Info->Doc)) Info->Doc=CopyStr(Info->Doc, "/");

    if (StrValid(Token)) Info->Port=atoi(Token);


    if (StrValid(Proto) && (CompareStr(Proto,"https")==0)) Info->Flags |= HTTP_SSL;

    ptr=GetNameValuePair(ptr,"\\S","=",&Token, &Value);
    while (ptr)
    {
        if (strcasecmp(Token, "oauth")==0)
        {
            Info->AuthFlags |= HTTP_AUTH_OAUTH;
            Info->Credentials=CopyStr(Info->Credentials, Value);
        }
        else if (strcasecmp(Token, "hostauth")==0) Info->AuthFlags |= HTTP_AUTH_HOST;
        else if (strcasecmp(Token, "http1.0")==0) Info->Flags |= HTTP_VER1_0;
        else if (strcasecmp(Token, "method")==0)   Info->Method=CopyStr(Info->Method, Value);
        else if (strcasecmp(Token, "content-type")==0)   Info->PostContentType=CopyStr(Info->PostContentType, Value);
        else if (strcasecmp(Token, "content-length")==0) Info->PostContentLength=atoi(Value);
        else if (strcasecmp(Token, "useragent")==0) Info->UserAgent=CopyStr(Info->UserAgent, Value);
        else if (strcasecmp(Token, "user-agent")==0) Info->UserAgent=CopyStr(Info->UserAgent, Value);
        else if (strcasecmp(Token, "user")==0) User=CopyStr(User, Value);
        else if (strcasecmp(Token, "password")==0) Pass=CopyStr(Pass, Value);
        else if (strcasecmp(Token, "keepalive")==0) Info->Flags |= HTTP_KEEPALIVE;
        else if (strcasecmp(Token, "timeout")==0) Info->Timeout=atoi(Value);
        else if (strcasecmp(Token, "authtype")==0)
        {
            if (strcasecmp(Value, "digest")==0) Info->AuthFlags |= HTTP_AUTH_DIGEST;
        }
        else if (strcasecmp(Token, "digest-auth")==0) Info->AuthFlags |= HTTP_AUTH_DIGEST;
        else SetVar(Info->CustomSendHeaders, Token, Value);
        ptr=GetNameValuePair(ptr,"\\S","=",&Token, &Value);
    }

    HTTPInfoPOSTSetContent(Info, "", Args, 0, 0);
    HTTPSetLoginCreds(Info, User, Pass);

    if (StrEnd(Info->Doc)) Info->Doc=CopyStr(Info->Doc, "/");

    DestroyString(User);
    DestroyString(Pass);
    DestroyString(Token);
    DestroyString(Value);
    DestroyString(Proto);
    DestroyString(Args);
    DestroyString(URL);
}


HTTPInfoStruct *HTTPInfoFromURL(const char *Method, const char *URL)
{
    HTTPInfoStruct *Info;
    Info=HTTPInfoCreate("HTTP/1.1","", 80, "", "", Method, "", "",0);
    HTTPInfoSetURL(Info, Method, URL);

    return(Info);
}






//Parse Cookies from Server. These are sent singly, and what comes after ';' is
//extra settings for that apply to this single cookie
static void HTTPParseServerCookie(const char *Str)
{
    char *Name=NULL, *Value=NULL;
    ListNode *Node;
    const char *ptr;


    if (! Cookies)
    {
        Cookies=ListCreate(LIST_FLAG_TIMEOUT);
        ListSetDestroyer(Cookies, Destroy);
    }

    ptr=GetNameValuePair(Str, ";", "=", &Name, &Value);
    StripTrailingWhitespace(Name);
    StripLeadingWhitespace(Name);
    StripTrailingWhitespace(Value);
    StripLeadingWhitespace(Value);
    Node=SetVar(Cookies, Name, Value);

    ptr=GetNameValuePair(ptr, ";", "=", &Name, &Value);
    while (ptr)
    {
        StripTrailingWhitespace(Name);
        StripLeadingWhitespace(Name);
        StripTrailingWhitespace(Value);
        StripLeadingWhitespace(Value);

        if (strcasecmp(Name, "expires")==0) ListNodeSetTime(Node, DateStrToSecs("%a, %d %b %Y %H:%M:%S", Value, NULL));
        if (strcasecmp(Name, "max-age")==0) ListNodeSetTime(Node, GetTime(TIME_CACHED) + atoi(Value));
        ptr=GetNameValuePair(ptr, ";", "=", &Name, &Value);
    }

    DestroyString(Name);
    DestroyString(Value);

}



char *HTTPClientAppendCookies(char *InStr, ListNode *CookieList)
{
    ListNode *Curr;
    char *Tempstr=NULL;
    time_t Expires, Now;

    Now=GetTime(TIME_CACHED);
    Tempstr=InStr;
    Curr=ListGetNext(CookieList);

    if (Curr)
    {
        Tempstr=CatStr(Tempstr,"Cookie: ");
        while ( Curr )
        {
            Expires=ListNodeGetTime(Curr);
            if ((Expires == 0) || (Expires < Now))
            {
                Tempstr=MCatStr(Tempstr, Curr->Tag, "=", (char *) Curr->Item, NULL);
                if (Curr->Next) Tempstr=CatStr(Tempstr, "; ");
            }
            Curr=ListGetNext(Curr);
        }
        Tempstr=CatStr(Tempstr,"\r\n");
    }

    return(Tempstr);
}


void HTTPClearCookies()
{
    ListClear(Cookies, Destroy);
}


static int HTTPHandleWWWAuthenticate(const char *Line, int *Type, char **Config)
{
    const char *ptr, *ptr2;
    char *Token=NULL, *Name=NULL, *Value=NULL;
    char *Realm=NULL, *QOP=NULL, *Nonce=NULL, *Opaque=NULL, *AuthType=NULL;

    ptr=Line;
    while (isspace(*ptr)) ptr++;
    ptr=GetToken(ptr," ",&Token,0);

    *Type &= ~(HTTP_AUTH_BASIC | HTTP_AUTH_DIGEST);
    if (strcasecmp(Token,"basic")==0) *Type |= HTTP_AUTH_BASIC;
    if (strcasecmp(Token,"digest")==0) *Type |= HTTP_AUTH_DIGEST;

    QOP=CopyStr(QOP,"");
    Realm=CopyStr(Realm,"");
    Nonce=CopyStr(Nonce,"");
    Opaque=CopyStr(Opaque,"");

    while (ptr)
    {
        ptr=GetToken(ptr,",",&Token,GETTOKEN_QUOTES);
        StripLeadingWhitespace(Token);
        StripTrailingWhitespace(Token);
        ptr2=GetToken(Token,"=",&Name,GETTOKEN_QUOTES);
        ptr2=GetToken(ptr2,"=",&Value,GETTOKEN_QUOTES);


        if (strcasecmp(Name,"realm")==0) Realm=CopyStr(Realm,Value);
        else if (strcasecmp(Name,"qop")==0)  QOP=CopyStr(QOP,Value);
        else if (strcasecmp(Name,"nonce")==0) Nonce=CopyStr(Nonce,Value);
        else if (strcasecmp(Name,"opaque")==0) Opaque=CopyStr(Opaque,Value);
    }

    //put all the digest parts into a single string that we can parse out later
    //THIS IS NOT CONSTRUCTING OUR REPLY TO THE DIGEST AUTH REQUEST, it is just storing data for later use
    if (*Type & HTTP_AUTH_DIGEST) *Config=MCopyStr(*Config, Realm,":", Nonce, ":", QOP, ":", Opaque, ":", NULL);
    else *Config=MCopyStr(*Config,Realm,":",NULL);

    DestroyString(Token);
    DestroyString(Name);
    DestroyString(Value);
    DestroyString(Realm);
    DestroyString(QOP);
    DestroyString(Nonce);
    DestroyString(Opaque);
    DestroyString(AuthType);

    return(*Type);
}

/* redirect headers can get complex. WE can have a 'full' header:
 *    http://myhost/whatever
 * or just the 'path' part
 *    /whatever
 * or even this
 *    //myhost/whatever
 */

static void HTTPParseLocationHeader(HTTPInfoStruct *Info, const char *Header)
{
    if (
        (strncasecmp(Header,"http:",5)==0) ||
        (strncasecmp(Header,"https:",6)==0)
    )
    {
        Info->RedirectPath=HTTPQuoteChars(Info->RedirectPath, Header, " ");
    }
    else if (strncmp(Header, "//",2)==0)
    {
        if (Info->Flags & HTTP_SSL) Info->RedirectPath=MCopyStr(Info->RedirectPath,"https:",Header,NULL);
        else Info->RedirectPath=MCopyStr(Info->RedirectPath,"http:",Header,NULL);
    }
    else
    {
        if (Info->Flags & HTTP_SSL) Info->RedirectPath=FormatStr(Info->RedirectPath,"https://%s:%d%s",Info->Host,Info->Port,Header);
        else Info->RedirectPath=FormatStr(Info->RedirectPath,"http://%s:%d%s",Info->Host,Info->Port,Header);
    }

}


static void HTTPParseHeader(STREAM *S, HTTPInfoStruct *Info, char *Header)
{
    char *Token=NULL, *Tempstr=NULL;
    const char *ptr;

    if (Info->Flags & HTTP_DEBUG) fprintf(stderr,"HEADER: %s\n",Header);
    ptr=GetToken(Header,":",&Token,0);
    while (isspace(*ptr)) ptr++;

    Tempstr=MCopyStr(Tempstr,"HTTP:",Token,NULL);
    STREAMSetValue(S,Tempstr,ptr);
    ListAddNamedItem(Info->ServerHeaders,Token,CopyStr(NULL,ptr));

    if (StrValid(Token) && StrValid(ptr))
    {
        switch (*Token)
        {
        case 'C':
        case 'c':
            if (strcasecmp(Token,"Content-length")==0)
            {
                Info->ContentLength=atoi(ptr);
                if (S->Size ==0) S->Size=strtoul(ptr, NULL, 10);
            }
            else if (strcasecmp(Token,"Content-type")==0)
            {
                Info->ContentType=CopyStr(Info->ContentType,ptr);
            }
            else if (strcasecmp(Token,"Connection")==0)
            {
                if (strcasecmp(ptr,"Close")==0) Info->Flags &= ~HTTP_KEEPALIVE;
            }
            else if ((strcasecmp(Token,"Content-Encoding")==0) )
            {
                if (! (Info->Flags & HTTP_NODECODE))
                {
                    if (
                        (strcasecmp(ptr,"gzip")==0) ||
                        (strcasecmp(ptr,"x-gzip")==0)
                    )
                    {
                        Info->Flags |= HTTP_GZIP;
                    }
                    if (
                        (strcasecmp(ptr,"deflate")==0)
                    )
                    {
                        Info->Flags |= HTTP_DEFLATE;
                    }
                }
            }
            break;

        case 'D':
        case 'd':
            if (strcasecmp(Token,"Date")==0) Info->Timestamp=CopyStr(Info->Timestamp,ptr);
            break;

        case 'L':
        case 'l':
            if (strcasecmp(Token,"Location")==0) HTTPParseLocationHeader(Info, ptr);
            break;

        case 'P':
        case 'p':
            if (strcasecmp(Token,"Proxy-Authenticate")==0) HTTPHandleWWWAuthenticate(ptr, &Info->AuthFlags, &Info->ProxyAuthorization);
            break;

        case 'W':
        case 'w':
            if (strcasecmp(Token,"WWW-Authenticate")==0) HTTPHandleWWWAuthenticate(ptr, &Info->AuthFlags, &Info->Authorization);
            break;

        case 'S':
        case 's':
            if (strcasecmp(Token,"Set-Cookie")==0) HTTPParseServerCookie(ptr);
            else if (strcasecmp(Token,"Status")==0)
            {
                //'Status' overrides the response
                Info->ResponseCode=CopyStrLen(Info->ResponseCode,ptr,3);
                STREAMSetValue(S,"HTTP:ResponseCode",Info->ResponseCode);
                STREAMSetValue(S,"HTTP:ResponseReason",Tempstr);
            }
            break;


        case 'T':
        case 't':
            if (
                (strcasecmp(Token,"Transfer-Encoding")==0)
            )
            {
                if (! (Info->Flags & HTTP_NODECODE))
                {
                    if (strstr(ptr,"chunked"))
                    {
                        Info->Flags |= HTTP_CHUNKED;
                    }
                }
            }
            break;
        }
    }

    DestroyString(Token);
    DestroyString(Tempstr);
}


static char *HTTPDigest(char *RetStr, const char *Method, const char *Logon, const char *Password, const char *Doc, const char *AuthInfo)
{
    char *Tempstr=NULL, *HA1=NULL, *HA2=NULL, *Digest=NULL;
    char *Realm=NULL, *Nonce=NULL, *QOP=NULL, *Opaque=NULL, *ClientNonce=NULL;
    const char *ptr;
    int len1, len2;
    static unsigned long AuthCounter=0;


    //if (*Type & HTTP_AUTH_DIGEST) *Config=MCopyStr(*Config, Realm,":", Nonce, ":", QOP, ":", Opaque, ":", NULL);

    ptr=GetToken(AuthInfo, ":", &Realm, GETTOKEN_QUOTES);
    ptr=GetToken(ptr, ":", &Nonce, GETTOKEN_QUOTES);
    ptr=GetToken(ptr, ":", &QOP, GETTOKEN_QUOTES);
    ptr=GetToken(ptr, ":", &Opaque, GETTOKEN_QUOTES);

    Tempstr=FormatStr(Tempstr,"%s:%s:%s",Logon,Realm,Password);
    len1=HashBytes(&HA1,"md5",Tempstr,StrLen(Tempstr),ENCODE_HEX);

    Tempstr=FormatStr(Tempstr,"%s:%s",Method,Doc);
    len2=HashBytes(&HA2,"md5",Tempstr,StrLen(Tempstr),ENCODE_HEX);


    if (strcmp(QOP, "auth")==0)
    {
        AuthCounter++;
        ClientNonce=GetRandomAlphabetStr(ClientNonce, 16);
        Tempstr=FormatStr(Tempstr,"%s:%s:%08d:%s:auth:%s",HA1,Nonce,AuthCounter,ClientNonce,HA2);
        len2=HashBytes(&Digest,"md5",Tempstr,StrLen(Tempstr),ENCODE_HEX);
        RetStr=FormatStr(RetStr,"username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", qop=\"auth\", nc=\"%08d\", cnonce=\"%s\", response=\"%s\"",Logon,Realm,Nonce,Doc,AuthCounter,ClientNonce,Digest);
    }
    else
    {
        Tempstr=MCopyStr(Tempstr,HA1,":",Nonce,":",HA2,NULL);
        len2=HashBytes(&Digest,"md5",Tempstr,StrLen(Tempstr),ENCODE_HEX);
        RetStr=MCopyStr(RetStr, "username=\"",Logon,"\", realm=\"",Realm,"\", nonce=\"",Nonce,"\", response=\"",Digest,"\", ","uri=\"",Doc,"\", algorithm=\"MD5\"", NULL);
    }




    DestroyString(Tempstr);
    DestroyString(HA1);
    DestroyString(HA2);
    DestroyString(Digest);
    DestroyString(ClientNonce);
    DestroyString(Realm);
    DestroyString(Nonce);
    DestroyString(QOP);
    DestroyString(Opaque);

    return(RetStr);
}





static char *HTTPHeadersAppendAuth(char *RetStr, const char *AuthHeader, HTTPInfoStruct *Info, const char *AuthInfo)
{
    char *SendStr=NULL, *Tempstr=NULL, *Realm=NULL, *Nonce=NULL;
    const char *ptr;
    char *Logon=NULL;
    const char *p_Password=NULL;
    int len, passlen;

    if (StrEnd(AuthInfo)) return(RetStr);
    if (Info->AuthFlags & (HTTP_AUTH_TOKEN | HTTP_AUTH_OAUTH)) return(MCatStr(RetStr, AuthHeader,": ",AuthInfo,"\r\n",NULL));

    SendStr=CatStr(RetStr,"");

    ptr=GetToken(AuthInfo,":",&Realm,0);
    ptr=GetToken(ptr,":",&Nonce,0);

    passlen=CredsStoreLookup(Realm, Info->UserName, &p_Password);
    if (! passlen)
    {
        Tempstr=FormatStr(Tempstr, "https://%s:%d",Info->Host, Info->Port);
        passlen=CredsStoreLookup(Tempstr, Info->UserName, &p_Password);
    }
    if (! passlen) passlen=CredsStoreLookup(Info->Host, Info->UserName, &p_Password);
    if (!passlen)
    {
        p_Password=Info->Credentials;
        passlen=StrLen(Info->Credentials);
    }


    if (passlen)
    {
        if (Info->AuthFlags & HTTP_AUTH_DIGEST)
        {
            Tempstr=HTTPDigest(Tempstr, Info->Method, Info->UserName, p_Password, Info->Doc, AuthInfo);
            SendStr=MCatStr(SendStr,AuthHeader,": Digest ", Tempstr, "\r\n",NULL);
        }
        else
        {
            Tempstr=MCopyStr(Tempstr,Info->UserName,":",NULL);
            //Beware! Password will not be null terminated
            Tempstr=CatStrLen(Tempstr,p_Password,passlen);
            len=strlen(Tempstr);

            //We should now have Logon:Password
            Nonce=SetStrLen(Nonce,len * 2);

            Nonce=EncodeBytes(Nonce, Tempstr, len, ENCODE_BASE64);
            SendStr=MCatStr(SendStr,AuthHeader,": Basic ",Nonce,"\r\n",NULL);

            //wipe Tempstr, because it held password for a while
            xmemset(Tempstr,0,len);
        }

    }

    //even if we didn't send the password, say we did so here in order that
    //we're not stuck in an eternal loop of sending passwords
    Info->AuthFlags |= HTTP_AUTH_SENT;

    DestroyString(Tempstr);
    DestroyString(Logon);
    DestroyString(Realm);
    DestroyString(Nonce);

    return(SendStr);
}


void HTTPSendHeaders(STREAM *S, HTTPInfoStruct *Info)
{
    char *SendStr=NULL, *Tempstr=NULL;
    ListNode *Curr;

    STREAMClearDataProcessors(S);
    SendStr=CopyStr(SendStr,Info->Method);
    SendStr=CatStr(SendStr," ");

    if (Info->Flags & HTTP_PROXY) Tempstr=HTTPInfoToURL(Tempstr, Info);
    else Tempstr=HTTPQuoteChars(Tempstr,Info->Doc," ");
    SendStr=CatStr(SendStr,Tempstr);

    if (Info->Flags & HTTP_VER1_0) SendStr=CatStr(SendStr," HTTP/1.0\r\n");
    else SendStr=MCatStr(SendStr," ",Info->Protocol,"\r\n","Host: ",Info->Host,"\r\n",NULL);

    if (StrValid(Info->PostContentType))
    {
        Tempstr=FormatStr(Tempstr,"Content-type: %s\r\n",Info->PostContentType);
        SendStr=CatStr(SendStr,Tempstr);
    }

    //probably need to find some other way of detecting need for sending ContentLength other than whitelisting methods
    if ((Info->PostContentLength > 0) &&
            (
                (strcasecmp(Info->Method,"POST")==0) ||
                (strcasecmp(Info->Method,"PUT")==0) ||
                (strcasecmp(Info->Method,"PROPFIND")==0) ||
                (strcasecmp(Info->Method,"PROPPATCH")==0) ||
                (strcasecmp(Info->Method,"PATCH")==0)
            )
       )
    {
        Tempstr=FormatStr(Tempstr,"Content-Length: %d\r\n",Info->PostContentLength);
        SendStr=CatStr(SendStr,Tempstr);
    }

    if (StrValid(Info->Destination))
    {
        Tempstr=FormatStr(Tempstr,"Destination: %s\r\n",Info->Destination);
        SendStr=CatStr(SendStr,Tempstr);
    }


    /* If we have authorisation details then send them */
    if (Info->AuthFlags & HTTP_AUTH_OAUTH)
    {
        Info->Authorization=MCopyStr(Info->Authorization, "Bearer ", OAuthLookup(Info->Credentials, FALSE), NULL);
    }

    if (StrValid(Info->Authorization)) SendStr=HTTPHeadersAppendAuth(SendStr, "Authorization", Info, Info->Authorization);
    else if (Info->AuthFlags & (HTTP_AUTH_HOST | HTTP_AUTH_BASIC | HTTP_AUTH_DIGEST)) SendStr=HTTPHeadersAppendAuth(SendStr, "Authorization", Info, Info->Host);

    if (Info->ProxyAuthorization) SendStr=HTTPHeadersAppendAuth(SendStr, "Proxy-Authorization", Info, Info->ProxyAuthorization);

    if (Info->Flags & HTTP_NOCACHE) SendStr=CatStr(SendStr,"Pragma: no-cache\r\nCache-control: no-cache\r\n");


    if (Info->Depth > 0)
    {
        Tempstr=FormatStr(Tempstr,"Depth: %d\r\n",Info->Depth);
        SendStr=CatStr(SendStr,Tempstr);
    }

    /*
    if ((PathData->Options.Restart) && (PathData->offset >0))
    {
    snprintf(Buffer,sizeof(Buffer),"Range: bytes=%d-\r\n",PathData->offset);
    SendStr=CatStr(SendStr,Buffer);

    }
    */

    if (Info->IfModifiedSince > 0)
    {
        Tempstr=CopyStr(Tempstr,GetDateStrFromSecs("%a, %d %b %Y %H:%M:%S GMT",Info->IfModifiedSince,NULL));
        SendStr=MCatStr(SendStr,"If-Modified-Since: ",Tempstr, "\r\n",NULL);
    }

    if (
        (strcasecmp(Info->Method,"DELETE") !=0) &&
        (strcasecmp(Info->Method,"HEAD") !=0) &&
        (strcasecmp(Info->Method,"PUT") !=0)
    )
    {

        Tempstr=CopyStr(Tempstr,"");

        if (! (Info->Flags & HTTP_NOCOMPRESS))
        {
            if (DataProcessorAvailable("compress","gzip")) Tempstr=CatStr(Tempstr,"gzip");
            if (DataProcessorAvailable("compress","zlib"))
            {
                if (StrValid(Tempstr)) Tempstr=CatStr(Tempstr,", deflate");
                else Tempstr=CatStr(Tempstr,"deflate");
            }
        }

        if (StrValid(Tempstr)) SendStr=MCatStr(SendStr,"Accept-Encoding: ",Tempstr,"\r\n",NULL);
    }

    if (Info->Flags & HTTP_KEEPALIVE)
    {
        //if (Info->Flags & HTTP_VER1_0)
        SendStr=CatStr(SendStr,"Connection: Keep-Alive\r\n");
        //SendStr=CatStr(SendStr,"Content-Length: 0\r\n");
    }
    else
    {
        SendStr=CatStr(SendStr,"Connection: close\r\n");
    }

    SendStr=MCatStr(SendStr,"User-Agent: ",Info->UserAgent, "\r\n",NULL);

    Curr=ListGetNext(Info->CustomSendHeaders);
    while (Curr)
    {
        SendStr=MCatStr(SendStr,Curr->Tag, ": ", (char *)  Curr->Item, "\r\n",NULL);
        Curr=ListGetNext(Curr);
    }

    if (! (Info->Flags & HTTP_NOCOOKIES))
    {
        SendStr=HTTPClientAppendCookies(SendStr,Cookies);
    }

    SendStr=CatStr(SendStr,"\r\n");

    Info->State |= HTTP_HEADERS_SENT;
    if (Info->Flags & HTTP_DEBUG) fprintf(stderr,"HTTPSEND: ------\n%s------\n\n",SendStr);
    STREAMWriteLine(SendStr,S);

//this flush pushes the data to the server, which is needed for most HTTP methods
//other than PUT or POST because we'll send no further data. It has a useful
//side-effect that the STREAM buffers will be wiped of any lingering data, which
//is important as we've just sent headers that could include passwords or other
//sensitive data
    STREAMFlush(S);

//wipe send str, because headers can contain sensitive data like passwords
    xmemset(SendStr,0,StrLen(SendStr));

    DestroyString(Tempstr);
    DestroyString(SendStr);
}




void HTTPReadHeaders(STREAM *S, HTTPInfoStruct *Info)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

    ListClear(Info->ServerHeaders,Destroy);
    Info->ContentLength=0;

//Not needed
    Info->RedirectPath=CopyStr(Info->RedirectPath,"");
    Info->Flags &= ~(HTTP_CHUNKED | HTTP_GZIP | HTTP_DEFLATE);
    Tempstr=STREAMReadLine(Tempstr,S);
    if (Tempstr)
    {
        if (Info->Flags & HTTP_DEBUG) fprintf(stderr,"RESPONSE: %s\n",Tempstr);
        //Token will be protocol (HTTP/1.0 or ICY or whatever)
        ptr=GetToken(Tempstr,"\\S",&Token,0);
        Info->ResponseCode=CopyStrLen(Info->ResponseCode,ptr,3);
        STREAMSetValue(S,"HTTP:ResponseCode",Info->ResponseCode);
        STREAMSetValue(S,"HTTP:ResponseReason",Tempstr);
        Tempstr=STREAMReadLine(Tempstr,S);
    }

    while (Tempstr)
    {
        StripTrailingWhitespace(Tempstr);
        if (StrEnd(Tempstr)) break;
        HTTPParseHeader(S, Info,Tempstr);
        Tempstr=STREAMReadLine(Tempstr,S);
    }

    S->BytesRead=0;
    ptr=STREAMGetValue(S, "HTTP:Content-Length");
    if (ptr) S->Size=(atoi(ptr));

    DestroyString(Tempstr);
    DestroyString(Token);
}



int HTTPProcessResponse(HTTPInfoStruct *HTTPInfo)
{
    int result=HTTP_ERROR;
    char *Proto=NULL, *Tempstr=NULL;
    int RCode;

    if (HTTPInfo->ResponseCode)
    {
        RCode=atoi(HTTPInfo->ResponseCode);
        switch (RCode)
        {
        case 304:
            result=HTTP_NOTMODIFIED;
            break;

        case 200:
        case 201:
        case 202:
        case 203:
        case 204:
        case 205:
        case 206:
        case 207:
        case 400:
            result=HTTP_OKAY;
            break;

        case 301:
        case 302:
        case 303:
        case 307:
        case 308:
            if (HTTPInfo->PreviousRedirect && (CompareStr(HTTPInfo->RedirectPath,HTTPInfo->PreviousRedirect)==0)) result=HTTP_CIRCULAR_REDIRECTS;
            else
            {
                if (HTTPInfo->Flags & HTTP_DEBUG) fprintf(stderr,"HTTP: Redirected to %s\n",HTTPInfo->RedirectPath);

                //As redirect check has been done, we can copy redirect path to previous
                HTTPInfo->PreviousRedirect=CopyStr(HTTPInfo->PreviousRedirect,HTTPInfo->RedirectPath);
                ParseURL(HTTPInfo->RedirectPath, &Proto, &HTTPInfo->Host, &Tempstr,NULL, NULL,&HTTPInfo->Doc,NULL);
                HTTPInfo->Port=atoi(Tempstr);

                //if HTTP_SSL_REWRITE is set, then all redirects get forced to https
                if (HTTPInfo->Flags & HTTP_SSL_REWRITE) Proto=CopyStr(Proto,"https");
                if (CompareStr(Proto,"https")==0) HTTPInfo->Flags |= HTTP_SSL;
                else HTTPInfo->Flags &= ~HTTP_SSL;

                //303 Redirects must be get!
                if (RCode==303)
                {
                    HTTPInfo->Method=CopyStr(HTTPInfo->Method,"GET");
                    HTTPInfo->PostData=CopyStr(HTTPInfo->PostData,"");
                    HTTPInfo->PostContentType=CopyStr(HTTPInfo->PostContentType,"");
                    HTTPInfo->PostContentLength=0;
                }

                if (! (HTTPInfo->Flags & HTTP_NOREDIRECT)) result=HTTP_REDIRECT;
                else result=HTTP_OKAY;
            }
            break;

        //401 Means authenticate, so it's not a pure error
        case 401:
            //407 Means authenticate with a proxy
            result=HTTP_AUTH_BASIC;
            break;

        case 407:
            result=HTTP_AUTH_PROXY;
            break;

        default:
            result=HTTP_NOTFOUND;
            break;
        }
    }

    if (result == HTTP_NOTFOUND)
    {
        RaiseError(0, "HTTP", STREAMGetValue(HTTPInfo->S, "HTTP:ResponseReason"));
    }

    DestroyString(Proto);
    DestroyString(Tempstr);

    return(result);
}



STREAM *HTTPSetupConnection(HTTPInfoStruct *Info, int ForceHTTPS)
{
    char *Proto=NULL, *Host=NULL, *URL=NULL, *Tempstr=NULL;
    int Port=0;
    STREAM *S;

    //proto in here will not be http/https but tcp/ssl/tls
    Proto=CopyStr(Proto,"tcp");
    if (Info->Flags & HTTP_PROXY)
    {
        ParseURL(Info->Proxy, &Proto, &Host, &Tempstr, NULL, NULL, NULL,NULL);
        Port=atoi(Tempstr);

        if (ForceHTTPS) Proto=CopyStr(Proto,"ssl");
    }
    else
    {
        Host=CopyStr(Host,Info->Host);
        Port=Info->Port;

        if (ForceHTTPS || (Info->Flags & HTTP_SSL)) Proto=CopyStr(Proto,"ssl");

        if (Port==0)
        {
            if (CompareStr(Proto,"ssl")==0) Port=443;
            else Port=80;
        }
    }

    if (StrValid(Info->ConnectionChain)) URL=FormatStr(URL,"%s|%s:%s:%d/",Info->ConnectionChain,Proto,Host,Port);
    else URL=FormatStr(URL,"%s:%s:%d/",Proto,Host,Port);


    //there's no data to send in a GET, so turn on quickack with 'q'
    if (strcasecmp(Info->Method,"GET")==0) Tempstr=FormatStr(Tempstr, "rwq timeout=%d", Info->Timeout);
    else Tempstr=FormatStr(Tempstr, "rw timeout=%d", Info->Timeout);

    //we cannot create Info->S if there is one, but we can't free/destroy it
    //thus we map it to 'S' and if we don't connect, we set 'S' to null and
    //return that.
    if (! Info->S) Info->S=STREAMCreate();
    S=Info->S;
    //must do this before STREAMConnect as otherwise we get 'hostname mismatch' errors during SSL setup
    //because the underlying system doesn't know what the URL is to checkt the hostname
    S->Path=FormatStr(S->Path,"%s://%s:%d/%s", Proto, Host, Port, Info->Doc);


    if (Info->Flags & HTTP_DEBUG) fprintf(stderr,"Connect: %s  Config: %s\n", URL, Tempstr);

    if (STREAMConnect(S, URL, Tempstr))
    {
        S->Type=STREAM_TYPE_HTTP;
    }
    else
    {
        RaiseError(ERRFLAG_ERRNO, "http", "failed to connect to %s:%d",Host,Port);
        S=NULL;
    }

    if (S) STREAMSetItem(S, "HTTP:InfoStruct", Info);

    DestroyString(Tempstr);
    DestroyString(Proto);
    DestroyString(Host);
    DestroyString(URL);

    return(S);
}



STREAM *HTTPConnect(HTTPInfoStruct *Info)
{
    STREAM *S=NULL;

    S=Info->S;

    //returns false if S is null, so is safe to call
    if (! STREAMIsConnected(S))
    {
        //if we require HTTPS or 'try first' HTTPS  is set, try that https first
        if ( (g_HTTPFlags & HTTP_REQ_HTTPS) || (g_HTTPFlags & HTTP_TRY_HTTPS)) S=HTTPSetupConnection(Info, TRUE);
        //if https isn't required, then we can try unencrypted HTTP
        if ( (! STREAMIsConnected(S)) && (! (g_HTTPFlags & HTTP_REQ_HTTPS)) ) S=HTTPSetupConnection(Info, FALSE);
    }

    if (S && (! (Info->State & HTTP_HEADERS_SENT)) ) HTTPSendHeaders(S,Info);

    return(S);
}



static void HTTPTransactSetupDataProcessors(HTTPInfoStruct *Info, STREAM *S)
{
    if (Info->Flags & HTTP_CHUNKED) HTTPAddChunkedProcessor(S);

    if (Info->Flags & HTTP_GZIP)
    {
        STREAMAddStandardDataProcessor(S,"uncompress","gzip","");
    }
    else if (Info->Flags & HTTP_DEFLATE)
    {
        STREAMAddStandardDataProcessor(S,"uncompress","zlib","");
    }

    if (Info->Flags & (HTTP_CHUNKED | HTTP_GZIP | HTTP_DEFLATE)) STREAMReBuildDataProcessors(S);
}


static int HTTPTransactHandleAuthRequest(HTTPInfoStruct *Info, int AuthResult)
{
    switch (AuthResult)
    {
    //here this flag just means the server asked us to authenticate, not specifically that it asked
    //for basic authentication
    case HTTP_AUTH_BASIC:

        //if we're using OAUTH then try doing a refresh to get new creds.
        //Set a flag (HTTP_AUTH_RETURN) that means we'll give up if we fail again
        if (Info->AuthFlags & HTTP_AUTH_OAUTH)
        {
            //if HTTP_AUTH_RETURN is set, then we already tried getting a refresh
            if (Info->AuthFlags & HTTP_AUTH_RETURN) return(FALSE);
            Info->Authorization=MCopyStr(Info->Authorization, "Bearer ", OAuthLookup(Info->Credentials, TRUE), NULL);
        }
        //for normal authentication, if we've sent the authentication, or if we have no auth details, then give up
        else if (
            //(Info->AuthFlags & HTTP_AUTH_SENT) ||
            (Info->AuthFlags & HTTP_AUTH_RETURN) ||
            (! StrValid(Info->Authorization))
        ) return(FALSE);
        break;


    //if we got asked for proxy authentication bu have no auth details, then give up
    case HTTP_AUTH_PROXY:
        if (! StrValid(Info->ProxyAuthorization)) return(FALSE);
        break;
    }
    Info->AuthFlags |= HTTP_AUTH_RETURN;

    //if we get here then there was no questions raised about authentication!
    return(TRUE);
}


STREAM *HTTPTransact(HTTPInfoStruct *Info)
{
    int result=HTTP_NOCONNECT;
    STREAM *S=NULL;

    //we cannot destroy Info->S within this function, as it may be used by functions outside of this one
    //so we map it to 'S' and set 'S' to null if connection fails and return that
    while (1)
    {
        S=HTTPConnect(Info);

        if (STREAMIsConnected(S))
        {
            Info->ResponseCode=CopyStr(Info->ResponseCode,"");

            if (! (Info->State & HTTP_CLIENTDATA_SENT))
            {
                //Set this even if no client data to send, so we know we've been through here once
                Info->State |= HTTP_CLIENTDATA_SENT;

                if (StrValid(Info->PostData))
                {
                    STREAMWriteLine(Info->PostData, S);
                    if (Info->Flags & HTTP_DEBUG) fprintf(stderr,"\n%s\n",Info->PostData);
                }
                else
                {
                    if (strcasecmp(Info->Method,"POST")==0) break;
                    if (strcasecmp(Info->Method,"PUT")==0) break;
                    if (strcasecmp(Info->Method,"PATCH")==0) break;
                    if (strcasecmp(Info->Method,"PROPFIND")==0) break;
                    if (strcasecmp(Info->Method,"PROPPATCH")==0) break;
                }
            }


            //Must clear this once the headers and clientdata sent
            Info->State=0;

            HTTPReadHeaders(S, Info);
            result=HTTPProcessResponse(Info);

            //we got redirected somewhere else. Shutdown the current stream and go around again
            //this time our URL will be the redirected url
            if (result==HTTP_REDIRECT)
            {
                STREAMShutdown(S);
                //do not use STREAMDestroy, as S can be used by functions outside this one
                //STREAMDestroy(S);
                continue;
            }

            //if this returns FALSE, then the server asked us for authentication details and we have nay
            //but if it returns true it either means that the server didn't ask for this, or else that
            //we're using something like OAuth, in which case this function will try to refresh our auth
            //credentials, and return TRUE which means 'try again now'
            if ((result == HTTP_AUTH_BASIC) || (result == HTTP_AUTH_PROXY) )
            {
                if (HTTPTransactHandleAuthRequest(Info, result))
                {
                    STREAMShutdown(S);
                    //do not use STREAMDestroy, as S can be used by functions outside this one
                    //STREAMDestroy(S);
                    continue;
                }
                else break;
            }

            //this means we got redirected back to the page we just asked for! this is bad and could
            //put us in a loop, so we just give up before doing anything else
            if (result == HTTP_CIRCULAR_REDIRECTS) break;


            //tranaction succeeded, stop trying, break out of loop
            if (result == HTTP_OKAY) break;
            //any of these mean the tranaction failed. give up. break out of loop
            if (result == HTTP_NOTFOUND) break;
            if (result == HTTP_NOTMODIFIED) break;
            if (result == HTTP_ERROR) break;



            //if we get here then we didn't get a successful http connection, so we set S to null
            S=NULL;
        }
        else break;
    }

    //add data processors to deal with chunked data, gzip compression etc, because even if the
    if (S)
    {
        STREAMSetValue(S,"HTTP:URL",Info->Doc);
        HTTPTransactSetupDataProcessors(Info, S);
    }

    return(S);
}



STREAM *HTTPMethod(const char *Method, const char *URL, const char *ContentType, const char *ContentData, int ContentLength)
{
    HTTPInfoStruct *Info;
    STREAM *S;

    Info=HTTPInfoFromURL(Method, URL);
    HTTPInfoPOSTSetContent(Info, ContentType, ContentData, ContentLength, HTTP_POSTARGS);
    S=HTTPTransact(Info);
    return(S);
}


STREAM *HTTPGet(const char *URL)
{
    return(HTTPMethod("GET", URL, "","",0));
}


STREAM *HTTPPost(const char *URL, const char *ContentType, const char *Content)
{
    return(HTTPMethod("POST", URL, ContentType, Content, StrLen(Content)));
}


STREAM *HTTPWithConfig(const char *URL, const char *Config)
{
    char *Token=NULL;
    const char *ptr, *cptr, *p_Method="GET";
    STREAM *S;


    ptr=GetToken(Config,"\\S",&Token, 0);

//if the first arg contains '=' then they've forgotten to supply a method specifier, so go with 'GET' and
//treat all the args as name=value pairs that are dealt with in HTTPInfoSetURL
    if (strchr(Token,'=')) ptr=Config;
    else
    {
        for (cptr=Token; *cptr !='\0'; cptr++)
        {
            switch(*cptr)
            {
            case 'w':
                p_Method="POST";
                break;
            case 'W':
                p_Method="PUT";
                break;
            case 'P':
                p_Method="PATCH";
                break;
            case 'D':
                p_Method="DELETE";
                break;
            case 'H':
                p_Method="HEAD";
                break;
            }
        }
    }

    Token=MCopyStr(Token, URL, " ", ptr, NULL);
    S=HTTPMethod(p_Method, Token, "","", 0);

    DestroyString(Token);

    return(S);
}





int HTTPCopyToSTREAM(STREAM *Con, STREAM *S)
{
    const char *ptr;
    size_t size=0;

    //do not check response code of HTTP server streams (strictly speaking STREAM_TYPE_HTTP_ACCEPT)
    if (S->Type == STREAM_TYPE_HTTP)
    {
        ptr=STREAMGetValue(Con, "HTTP:ResponseCode");
        if ((! ptr) || (*ptr !='2'))
        {
            STREAMClose(Con);
            return(-1);
        }
    }

    ptr=STREAMGetValue(Con, "HTTP:Content-Length");
    if (StrValid(ptr)) size=strtol(ptr, NULL, 10);

    return(STREAMSendFile(Con, S, size, SENDFILE_LOOP));
}


int HTTPDownload(char *URL, STREAM *S)
{
    STREAM *Con;

    Con=HTTPGet(URL);
    if (! Con) return(-1);
    return(HTTPCopyToSTREAM(Con, S));
}


void HTTPSetFlags(int Flags)
{
    g_HTTPFlags=Flags;
}

int HTTPGetFlags()
{
    return(g_HTTPFlags);
}



int HTTPConnectOkay(STREAM *S)
{
    const char *ptr;

    if (! S) return(FALSE);
    ptr=STREAMGetValue(S, "HTTP:ResponseCode");
    if (! ptr) return(FALSE);

    if (*ptr=='2') return(TRUE);
    return(FALSE);
}
