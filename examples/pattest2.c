#include "../libUseful.h"


main()
{
    const char *Scans[]={"4206013920809400108106245948980785","1001908763470006013900399866666775","9001908763470006013900399866666775","10019W8763470006013900399866666775","41474633443700344454360003003349405723","00040479645004232891", "1001910563460006013900399858878386", NULL};
    int i, result, len;


    for (i=0; Scans[i] !=NULL; i++)
    {
	len=StrLen(Scans[i]);
	result=pmatch_one("[0-8]\\D{13}60139*", Scans[i], len, NULL, NULL, 0);
	printf("MATCH: %d %s\n", result, Scans[i]);
    }

    exit(0);

}
