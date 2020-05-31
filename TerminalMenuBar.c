#include "TerminalMenuBar.h"

void TerminalMenuBarUpdate(TERMMENU *MB)
{
    ListNode *Curr;
    char *Tempstr=NULL;

    if (MB->Flags & TERMMENU_POSITIONED) TerminalCommand(TERM_CURSOR_MOVE, MB->x, MB->y, MB->Term);
		else Tempstr=CopyStr(Tempstr, "\r");

		if (StrValid(MB->Text)) Tempstr=CopyStr(Tempstr, MB->Text);

    Curr=ListGetNext(MB->Options);
    while (Curr)
    {
        if (MB->Options->Side==Curr)
        {
            Tempstr=MCatStr(Tempstr, MB->MenuCursorLeft, Curr->Tag, MB->MenuCursorRight, NULL);
        }
        else Tempstr=MCatStr(Tempstr, MB->MenuPadLeft,Curr->Tag, MB->MenuPadRight, NULL);

        Curr=ListGetNext(Curr);
    }

    TerminalPutStr(Tempstr, MB->Term);
		STREAMFlush(MB->Term);

    Destroy(Tempstr);
}


char *TerminalMenuBarOnKey(char *RetStr, TERMMENU *MB, int key)
{
ListNode *Curr;

RetStr=CopyStr(RetStr, "");

switch (key)
{
   case EOF:
			Destroy(RetStr);
			return(NULL);
   break;

   case '<':
   case TKEY_LEFT:
       Curr=ListGetPrev(MB->Options->Side);
       if (Curr) MB->Options->Side=Curr;
   break;

   case '>':
   case TKEY_RIGHT:
        Curr=ListGetNext(MB->Options->Side);
        if (Curr) MB->Options->Side=Curr;
        break;

   case '\r':
   case '\n':
        RetStr=CopyStr(RetStr, MB->Options->Side->Tag);
   break;
}

TerminalMenuBarUpdate(MB);

return(RetStr);
}



char *TerminalMenuBar(char *RetStr, STREAM *Term, const char *Config)
{
    ListNode *Items, *Curr;
		TERMMENU *MB;
    const char *ptr;
    int inchar;

    MB=TerminalMenuBarCreate(Term, Config);

    TerminalMenuBarUpdate(MB);
    inchar=TerminalReadChar(MB->Term);
    while (TRUE)
    {
			RetStr=TerminalMenuBarOnKey(RetStr, MB, inchar);

			//there's two scenarios where we're 'done' with the menubar. 
			//The first is if the user selects an item, in which case 'RetStr'
			//will have a length (and StrValid will return true).
			//The other case is if the user presses 'escape', in which case
			//RetStr will be NULL
      if ((! RetStr) || StrValid(RetStr)) break;
      inchar=TerminalReadChar(MB->Term);
    }

		TerminalMenuBarDestroy(MB);
    return(RetStr);
}



