#include "UnitsOfMeasure.h"

double ToPower(double val, double power)
{
    double result=0;
    int i;

    result=val;
    for (i=1; i < power; i++)
    {
        result=result * val;
    }

    return(result);
}


double FromSIUnit(const char *Data, int Base)
{
    double val;
    char *ptr=NULL;

    val=strtod(Data,&ptr);
    while (isspace(*ptr)) ptr++;
    switch (*ptr)
    {
    case 'k':
        val=val * Base;
        break;
    case 'M':
        val=val * ToPower(Base,2);
        break;
    case 'G':
        val=val * ToPower(Base,3);
        break;
    case 'T':
        val=val * ToPower(Base,4);
        break;
    case 'P':
        val=val * ToPower(Base,5);
        break;
    case 'E':
        val=val * ToPower(Base,6);
        break;
    case 'Z':
        val=val * ToPower(Base,7);
        break;
    case 'Y':
        val=val * ToPower(Base,8);
        break;
    }

    return(val);
}


//for each suffix calcuate the value of that suffix
//and see if it's greater than the value we are trying to format
//so if we have 4096 to format, we first test against 1024. is our
//value greater than that? If not we just use no suffix as 'k' is
//our minimum. Otherwize we test against 1024 ^ 1024 (Meg) and so on
int FindSuffix(const char *sufflist, double Value, int Base, int PreferFract, int Precision)
{
    double next=0, limit;
    int i;

    for (i=0; sufflist[i] !='\0'; i++)
    {
        next=ToPower(Base, i+1);
        if (next > Value) break;

        //if we have no next suffix, then stick with this one
        if (sufflist[i+1] == '\0') break;
    }


    //Prefer Fractional Suffix: if this is requested and we still have a suffix to 'go up to'
    //then check if our value is within the specified precision.
    if (PreferFract && (sufflist[i] != '\0') && (sufflist[i+1] != '\0') )
    {
        next=ToPower(Base, i+1);
        limit=ToPower(10, Precision);
        if ((next / Value) > limit) PreferFract=FALSE;

        if (PreferFract) i++;
    }


    return(i);
}


//'Base' here will be 1024 or 1000 for computing or scientific units (IEC or Metric/SI)
const char *ToSIUnit(double Value, int Base, int Precision)
{
    static char *Str=NULL;
    char *Fmt=NULL;
    double next;
//Set to 0 to keep valgrind happy
    int i=0;
    char suffix=' ', *sufflist=" kMGTPEZYRQ";
    int PreferFract=FALSE;

    if (Precision < 0)
    {
        Precision=0 - Precision;
        PreferFract=TRUE;
    }

    i=FindSuffix(sufflist, Value, Base, PreferFract, Precision);

    if ((i > 0) && (sufflist[i] !='\0'))
    {
        Value=Value / ToPower(Base, i);
        suffix=sufflist[i];
        Fmt=FormatStr(Fmt, "%%0.%df%%c", Precision);
        Str=FormatStr(Str,Fmt,(float) Value,suffix);
    }
    else
    {
        //here 'next' is the remainder, by casting 'Value' to a long we remove the
        //decimal component, then subtract from Value. This leaves us with *only*
        //the decimal places
        next=Value - (long) Value;
        if (Precision==0) Str=FormatStr(Str,"%ld",(long) Value);
        else
        {
            Fmt=FormatStr(Fmt, "%%0.%df", Precision);
            Str=FormatStr(Str,Fmt,(float) Value);
        }
    }


    DestroyString(Fmt);
    return(Str);
}

