#include "Inet.h"
#include "Http.h"
#include "GeneralFunctions.h"
#include "Markup.h"
#include "IPAddress.h"
#include "DataParser.h"

static STREAM *GeoLocateOpenURL(const char *URL, const char *LookupIP)
{
    ListNode *Config;
    char *Tempstr=NULL;
    STREAM *S;

    Config=ListCreate();
    SetVar(Config, "ip", LookupIP);
    Tempstr=SubstituteVarsInString(Tempstr, URL, Config, 0);

    S=HTTPGet(Tempstr);

    ListDestroy(Config, Destroy);
    Destroy(Tempstr);

    return(S);
}


char *ExtractFromWebpage(char *RetStr, const char *URL, const char *ExtractStr, int MinLength)
{
    char *Tempstr=NULL;
    const char *ptr;
    ListNode *Vars;
    STREAM *S;

    Vars=ListCreate();

    S=GeoLocateOpenURL(URL, "");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr,S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);
            StripLeadingWhitespace(Tempstr);

            if (StrLen(Tempstr) >=MinLength)
            {
                if (! StrValid(ExtractStr)) RetStr=CopyStr(RetStr,Tempstr);
                else
                {
                    ExtractVarsFromString(Tempstr,ExtractStr,Vars);
                    ptr=GetVar(Vars,"extract_item");
                    if (StrValid(ptr)) RetStr=CopyStr(RetStr,ptr);
                }
            }
            Tempstr=STREAMReadLine(Tempstr,S);
        }
        STREAMClose(S);
    }

    ListDestroy(Vars,(LIST_ITEM_DESTROY_FUNC) Destroy);
    DestroyString(Tempstr);

    StripTrailingWhitespace(RetStr);
    StripLeadingWhitespace(RetStr);

    return(RetStr);
}



static ListNode *IPInfoLookupGetServices(const char *Type)
{
    char *Tempstr=NULL, *Token=NULL, *URL=NULL;
    const char *ptr;
    ListNode *Services=NULL;
    int len;
    STREAM *S;

    Services=ListCreate();
    S=LibUsefulConfigFileOpen("ip-lookup.conf", "LIBUSEFUL_IPLOOKUP_FILE", "IPLookupFile");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            if (StrValid(Tempstr))
            {
                StripTrailingWhitespace(Tempstr);
                ptr=GetToken(Tempstr, " ", &URL, 0);
                if (CompareStrNoCase(Type, "geolocate")==0)
                {
                    if (strstr(Tempstr, " geolocate=y")) SetVar(Services, URL, ptr);
                }
                else SetVar(Services, URL, ptr);

                Tempstr=STREAMReadLine(Tempstr, S);
            }
        }
        STREAMClose(S);

        //pick a random start position, and to through all servers from that start
        len=ListSize(Services);
	if (len > 1) ListRotate(Services, (time(NULL) + rand()) % len);
    }
    else RaiseError(0, "IPInfoLookupGetServices", "Cannot find/open ip-lookup.conf config file");

    Destroy(Tempstr);
    Destroy(Token);
    Destroy(URL);

    return(Services);
}






int ProcessIPGeolocateService(const char *IP, const char *URL, const char *Settings, ListNode *Vars)
{
    ListNode *Config;
    PARSER *P;
    char *Tempstr=NULL;
    const char *ptr;
    const char *Fields[]= {"ip", "latitude", "longitude", "country", "countrycode", "region", "city", "isp", "asn", "timezone", NULL};
    STREAM *S;
    int RetVal=FALSE;
    int i;

    S=GeoLocateOpenURL(URL, IP);
    if (S != NULL)
    {
        Tempstr=STREAMReadDocument(Tempstr, S);
        STREAMClose(S);

        Config=VarsFromNameValueList(Settings, "\\S", "=");
        ptr=GetVar(Config, "format");

        if (StrValid(ptr))
        {
            P=ParserParseDocument(ptr, Tempstr);
            for (i=0; Fields[i] !=NULL; i++)
            {
                ptr=GetVar(Config, Fields[i]);
                if (! StrValid(ptr)) ptr=Fields[i];

                SetVar(Vars, Fields[i], ParserGetValue(P, ptr));
            }
            if ( StrValid(GetVar(Vars, "country")) ) RetVal=TRUE;
	    ParserItemsDestroy(P);
        }
    }

    ListDestroy(Config, Destroy);
    Destroy(Tempstr);

    return(RetVal);
}


int IPGeoLocate(const char *Host, ListNode *Vars)
{
    STREAM *S=NULL;
    char *Tempstr=NULL, *IP=NULL;
    ListNode *Services, *Curr;
    const char *ptr;
    int result=FALSE;

    if (! StrValid(Host)) return(FALSE);

    if (! IsIPAddress(Host)) IP=CopyStr(IP, LookupHostIP(Host));
    else IP=CopyStr(IP, Host);


    Services=IPInfoLookupGetServices("geolocate");
    Curr=ListGetNext(Services);
    while (Curr)
    {
        result=ProcessIPGeolocateService(IP, Curr->Tag, Curr->Item, Vars);
        if (result) break;
        Curr=ListGetNext(Curr);
    }

    ListDestroy(Services, Destroy);
    DestroyString(Tempstr);

    return(result);
}




static char *IPFromGeoLocate(char *RetStr, const char *URL, const char *Settings)
{
ListNode *Vars;

Vars=ListCreate();
if (ProcessIPGeolocateService("", URL, Settings, Vars))
{
	RetStr=CopyStr(RetStr, GetVar(Vars, "ip"));
}

ListDestroy(Vars, Destroy);

return(RetStr);
}



char *GetExternalIP(char *RetStr)
{
    ListNode *Services, *Curr;
    char *Token=NULL;
    const char *ptr;
    STREAM *S;

    RetStr=CopyStr(RetStr,"");

    Services=IPInfoLookupGetServices("");
    Curr=ListGetNext(Services);
    while (Curr)
    {
        ptr=GetToken(Curr->Tag, ",", &Token, 0);
	if (strstr((const char *) Curr->Item, "format=json")) RetStr=IPFromGeoLocate(RetStr, Curr->Tag, Curr->Item);
        else RetStr=ExtractFromWebpage(RetStr, Token, ptr, 4);
        if (StrValid(RetStr)) break;
        Curr=ListGetNext(Curr);
    }

    ListDestroy(Services, Destroy);
    Destroy(Token);

    return(RetStr);
}



