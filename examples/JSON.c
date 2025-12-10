#include "../libUseful.h"

main()
{
PARSER *P;
ListNode *Curr;
char *Tempstr=NULL;

P=ParserParseDocument("json", "{'this': 'this is some text','that': 'that is some text','the other': 1234, 'subitem': {'stuff in a subitem': 'whatever', 'contents': 'with contents'}, 'multilevel subitem': {'title': 'this is a multilevel subitem', 'contents': {'name': 'level2', 'value': 1234}}, array:['value1', 'value2', 'value3']}");


Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "json", P);
printf("EXPORT JSON:\n%s\n\n", Tempstr);

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "xml", P);
printf("EXPORT XML:\n%s\n\n", Tempstr);

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "cmon", P);
printf("EXPORT CMON:\n%s\n\n", Tempstr);

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "cmon+expand", P);
printf("EXPORT CMON EXPAND:\n%s\n\n", Tempstr);

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "cmon+collapse", P);
printf("EXPORT CMON COLLAPSE:\n%s\n\n", Tempstr);


printf("ARRAY: ");
Curr=ParserOpenItem(P, "array");
Curr=ListGetNext(Curr);
while (Curr)
{
printf("%s: %s\n", Curr->Tag, (const char *) Curr->Item);
Curr=ListGetNext(Curr);
}
printf("\n");

Destroy(Tempstr);
}
