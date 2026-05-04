#include "Backtrace.h"
#include <execinfo.h>


char *GetBacktrace(char *RetStr)
{
#ifdef HAVE_BACKTRACE
    void **BTFuncs=NULL;
    char **Strings=NULL;
    int size, i;

    RetStr=CopyStr(RetStr, "");
    BTFuncs=(void **) calloc(15, sizeof(void *));
    size = backtrace(BTFuncs, 15);

    if (size > 0)
    {
        Strings=backtrace_symbols(BTFuncs, size);

        if (Strings)
        {

            for (i=0; i < size; i++)
            {
                RetStr=MCatStr(RetStr, Strings[i], "\n", NULL);
            }
            free(Strings);
        }
    }

    free(BTFuncs);
//we do not need to free the internal strings

    return(RetStr);
#endif

    return(CopyStr(RetStr, "backtrace not supported"));
}


