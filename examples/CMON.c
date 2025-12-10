#include "../libUseful.h"

main(int argc, char *argv[])
{
PARSER *P;
char *Tempstr=NULL;

if (argc > 1) Tempstr=FileRead(Tempstr, argv[1]);
else Tempstr=CopyStr(Tempstr, "value1=first value\nvalue2='second value'\nvalue with spaces in name=whatever\nobject1: member1='first member' member2='2nd member'\nline_array [ line 1\nline 2\nbig long line 3\n]array[this thing; that thing, the other thing; whatever]\nanother line array\n [this\nthat\nthe other\n]\nanother object\n{\nmember 1=hello\nmember2=there\n}\n");


P=ParserParseDocument("cmon", Tempstr);

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "json", P);
printf("EXPORT JSON:\n%s\n\n", Tempstr);

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "xml", P);
printf("EXPORT XML:\n%s\n\n", Tempstr);

Tempstr=CopyStr(Tempstr, "");
Tempstr=ParserExport(Tempstr, "cmon", P);
printf("EXPORT CMON:\n%s\n\n", Tempstr);


Destroy(Tempstr);
}
