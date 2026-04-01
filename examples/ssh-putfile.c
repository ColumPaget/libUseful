#include "../libUseful.h"

//test running a command on an ssh server

main(int argc, const char *argv[])
{
    STREAM *S;
    char *Tempstr=NULL;

    if (argv < 2)
    {
        printf("Insufficient arguments: %s <ssh host>\n", argv[0]);
        exit(1);
    }

    Tempstr=MCopyStr(Tempstr, "ssh:", argv[1], "/putfile.test", NULL);
    S=STREAMOpen(Tempstr, "w");
    if (S)
    {
	STREAMWriteLine("Line1\r\n", S);
	STREAMWriteLine("Line2\r\n", S);
	STREAMFlush(S);
//        sleep(5);
/*
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);
            printf("%s\n", Tempstr);
            Tempstr=STREAMReadLine(Tempstr, S);
        }
*/
        STREAMClose(S);
    }
    else printf("ERROR: failed to connect to %s\n", Tempstr);

}
