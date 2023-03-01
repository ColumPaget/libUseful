ListNode *TermTheme=NULL;

ListNode *TerminalThemeInit()
{
    TermTheme=ListCreate();

    SetVar(TermTheme, "Menu:CursorLeft", ">");
    SetVar(TermTheme, "Menu:Selected", "*");

    SetVar(TermTheme, "Choice:CursorLeft", "<");
    SetVar(TermTheme, "Choice:CursorRight", ">");

    return(TermTheme);
}


const char *TerminalThemeGet(const char *Name)
{
    return(GetVar(TermTheme, Name));
}

const char *TerminalThemeSet(const char *Name, const char *Value)
{
    return(GetVar(TermTheme, Name));
}

void TerminalThemeApply(TerminalWidget *TW, const char *Type)
{
    char *Tempstr=NULL;

    Tempstr=MCopyStr(Tempstr, Type, ":Attribs", NULL);
    TW->Attribs=CopyStr(TW->Attribs, TerminalThemeGet(Tempstr));
    Tempstr=MCopyStr(Tempstr, Type, ":CursorAttribs", NULL);
    TW->CursorAttribs=CopyStr(TW->CursorAttribs, TerminalThemeGet(Tempstr));
    Tempstr=MCopyStr(Tempstr, Type, ":SelectedAttribs", NULL);
    TW->SelectedAttribs=CopyStr(TW->SelectedAttribs, TerminalThemeGet(Tempstr));
    Tempstr=MCopyStr(Tempstr, Type, ":CursorLeft", NULL);
    TW->CursorLeft=CopyStr(TW->CursorLeft, TerminalThemeGet(Tempstr));
    Tempstr=MCopyStr(Tempstr, Type, ":CursorRight", NULL);
    TW->CursorRight=CopyStr(TW->CursorRight, TerminalThemeGet(Tempstr));

    Destroy(Tempstr);
}
