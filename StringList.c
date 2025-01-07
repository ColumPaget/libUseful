#include "StringList.h"
#include "Tokenizer.h"
#include "includes.h"

int InStringList(const char *Item, const char *List, const char *Sep)
{
    const char *ptr;
    char *Token=NULL;
    int RetVal=FALSE;

    ptr=GetToken(List, Sep, &Token, 0);
    while (ptr)
    {
        if (strcmp(Item, Token)==0)
        {
            RetVal=TRUE;
            break;
        }
        ptr=GetToken(ptr, Sep, &Token, 0);
    }

    Destroy(Token);

    return(RetVal);
}


char *StringListGet(char *RetStr, const char *List, const char *Sep, int Pos)
{
    const char *ptr;
    char *Token=NULL;
    int count=0;

    RetStr=CopyStr(RetStr, "");
    ptr=GetToken(List, Sep, &Token, 0);
    while (ptr)
    {
        if (count==Pos) RetStr=CopyStr(RetStr, Token);
        count++;
        ptr=GetToken(ptr, Sep, &Token, 0);
    }

    Destroy(Token);
    return(RetStr);
}
