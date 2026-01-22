#include "../libUseful.h"

main()
{
char *Tempstr=NULL;
ListNode *Vars, *Curr;

Vars=ListCreate();
IPGeoLocate("1.1.1.1", Vars);

Curr=ListGetNext(Vars);
while (Curr)
{
printf("%s: %s\n", Curr->Tag, Curr->Item);
Curr=ListGetNext(Curr);
}

Destroy(Tempstr);
}
