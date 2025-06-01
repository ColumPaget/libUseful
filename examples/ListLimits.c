#include "../libUseful.h"


void Destroyer(void *Item)
{
printf("DESTROY: %s\n", (const char *) Item);
Destroy(Item);
}


main()
{
ListNode *Map;
char *Tempstr=NULL;
int i;

Map=MapCreate(128, 0);
ListSetMaxItems(Map, 3, Destroyer);

for (i=0; i < 100000; i++)
{
Tempstr=FormatStr(Tempstr, "%d", i);
ListAddNamedItem(Map, Tempstr, CopyStr(NULL,Tempstr));
}

MapDumpSizes(Map);

Destroy(Tempstr);
}
