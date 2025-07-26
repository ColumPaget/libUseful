#include "../libUseful.h"

main()
{
uint32_t IP, Masked;

IP=StrtoIP("255.255.255.255");
Masked=IP4MaskIP(IP, 16);
printf("/16: %s\n", IPtoStr(Masked));
Masked=IP4MaskIP(IP, 17);
printf("/17: %s\n", IPtoStr(Masked));
Masked=IP4MaskIP(IP, 24);
printf("/24: %s\n", IPtoStr(Masked));

printf("MATCH: %d \n", IP4StrMatchesMaskStr("192.168.1.20", "192.168.1.0/24"));
}
