#include "HttpServer.h"
#include "Tokenizer.h"
#include "String.h"

void HTTPServerParseClientCookies(ListNode *Vars, const char *Str)
{
    const char *ptr;
    char *Name=NULL, *Value=NULL, *Tempstr=NULL;
    ListNode *Item;

    ptr=GetNameValuePair(Str, ";", "=", &Name, &Value);
    while (ptr)
    {
        StripTrailingWhitespace(Name);
        StripLeadingWhitespace(Name);
        StripTrailingWhitespace(Value);
        StripLeadingWhitespace(Value);

        //prepend with 'cookie' so we can find it in a STREAM var ist
        Tempstr=MCopyStr(Tempstr, "cookie:", Name, NULL);
        SetVar(Vars, Tempstr, Value);
        ptr=GetNameValuePair(ptr, ";", "=", &Name, &Value);
    }

    DestroyString(Name);
    DestroyString(Value);
    DestroyString(Tempstr);
}


void HTTPServerParseAuthorization(ListNode *Vars, const char *Str)
{
    char *Token=NULL;
    const char *ptr;

    ptr=GetToken(Str, "\\S", &Token, 0);
    if (strcasecmp(Token, "basic")==0)
    {
        ptr=GetToken(ptr, "\\S", &Token, 0);
        SetVar(Vars, "Auth:Basic", Token);
    }
    else if (strcasecmp(Token, "bearer")==0)
    {
        ptr=GetToken(ptr, "\\S", &Token, 0);
        SetVar(Vars, "Auth:Bearer", Token);
    }

    Destroy(Token);
}


void HTTPServerParseClientHeaders(STREAM *S)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

    Tempstr=STREAMReadLine(Tempstr, S);
    StripTrailingWhitespace(Tempstr);
    ptr=GetToken(Tempstr, "\\S", &Token, 0);
    STREAMSetValue(S, "HTTP:Method", Token);
    STREAMSetValue(S, "HTTP:URL", ptr);

    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        StripTrailingWhitespace(Tempstr);
        if (! StrValid(Tempstr)) break;

        ptr=GetToken(Tempstr, ":", &Token, 0);
        StripTrailingWhitespace(Token);
        if (strcasecmp(Token, "Cookie:")==0) HTTPServerParseClientCookies(S->Values, ptr);
        else if (strcasecmp(Token, "Authorization")==0) HTTPServerParseAuthorization(S->Values, ptr);
        Tempstr=STREAMReadLine(Tempstr, S);
    }

    Destroy(Tempstr);
    Destroy(Token);
}

#define HEADER_SEEN_CONNECTION 1
#define HEADER_SEEN_DATE       2


void HttpServerSendHeaders(STREAM *S, int ResponseCode, const char *ResponseText, const char *Headers)
{
    char *Tempstr=NULL, *Output=NULL, *Hash=NULL;
    char *Name=NULL, *Value=NULL;
    const char *ptr;
    int OutputSeen=0;

    Output=FormatStr(Output, "HTTP/1.1 %03d %s\r\n", ResponseCode, ResponseText);
    ptr=GetNameValuePair(Headers, "\\S", "=", &Name, &Value);
    while (ptr)
    {
        if (strcasecmp(Name, "Connection")==0) OutputSeen |= HEADER_SEEN_CONNECTION;
        if (strcasecmp(Name, "Date")==0) OutputSeen |= HEADER_SEEN_DATE;
        Output=MCatStr(Output, Name, ":", Value, "\r\n", NULL);
        ptr=GetNameValuePair(ptr, "\\S", "=", &Name, &Value);
    }

    if (! (OutputSeen & HEADER_SEEN_CONNECTION)) Output=CatStr(Output, "Connection: close\r\n");
    if (! (OutputSeen & HEADER_SEEN_DATE)) Output=MCatStr(Output, "Date: %s\r\n", GetDateStr("%a, %d %b %Y %H:%M:%S GMT", "GMT"));

    Output=CatStr(Output, "\r\n");

    STREAMWriteLine(Output, S);
    STREAMFlush(S);

    Destroy(Output);
    Destroy(Tempstr);
    Destroy(Name);
    Destroy(Value);
    Destroy(Hash);
}
