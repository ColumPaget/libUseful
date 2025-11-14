#include "../libUseful.h"


main()
{
ListNode *Files, *Curr;
int count, i;

Files=ListCreate();
count=RecursiveFindFilesInPath("*.exe", "/home/user1", Files);

printf("Found: %d\n", count);

Curr=ListGetNext(Files);
while (Curr)
{
printf("  %s\n", Curr->Item);
Curr=ListGetNext(Curr);
}


ListDestroy(Files, Destroy);
}

