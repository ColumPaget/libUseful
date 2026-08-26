#include "libUseful-5/libUseful.h"

main()
{
STREAM *S;
char *Tempstr=NULL;

S=STREAMOpen("mcast:239.255.255.250:1900", "");
Tempstr=STREAMReadLine(Tempstr, S);
while (Tempstr)
{
StripTrailingWhitespace(Tempstr);
printf("%s\n", Tempstr);
Tempstr=STREAMReadLine(Tempstr, S);
}

STREAMClose(S);

Destroy(Tempstr);
}
