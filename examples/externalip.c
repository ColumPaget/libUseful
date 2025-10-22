#include "../libUseful.h"

main()
{
char *Tempstr=NULL;

Tempstr=GetExternalIP(Tempstr);
printf("EXTERNAL IP: %s\n", Tempstr);

Destroy(Tempstr);
}
