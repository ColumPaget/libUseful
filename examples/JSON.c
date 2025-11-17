#include "../libUseful.h"

main()
{
PARSER *P;
char *Tempstr=NULL;

P=ParserParseDocument("json", "{'this': 'this is some text','that': 'that is some text','the other': 1234, 'subitem': {'stuff in a subitem': 'whatever', 'contents': 'with contents'}, 'multilevel subitem': {'title': 'this is a multilevel subitem', 'contents': {'name': 'level2', 'value': 1234}}}");

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "json", P);
printf("EXPORT JSON:\n %s\n\n", Tempstr);;

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "xml", P);
printf("EXPORT XML:\n %s\n\n", Tempstr);;

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "cmon", P);
printf("EXPORT CMON:\n %s\n\n", Tempstr);;

Destroy(Tempstr);
}
