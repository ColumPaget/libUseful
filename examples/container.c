#include "../libUseful.h"


main()
{
ProcessApplyConfig("container nonet +mnt=/etc,/dev,/lib,/usr/lib,/bin,/usr/bin");
system("/bin/sh");
}
