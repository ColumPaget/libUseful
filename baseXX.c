#include "baseXX.h"



/* This is surely not the fastest baseXX decoder, it is written for understanding/legiblity, not speed.
 * first we convert baseXX into binary, as a string of '1' and '0' characters
 * Then we work through this string converting it into eight-bit values.
 * The reason it's done this way is that it's tough to handle 'overlapping bits' when dealing with characters
 * that represent 5-bit or 6-bit chunks of a string of 8-bit values, and the resulting code of trying to do it all
 * with bit-shifting is rather unreadable
 */

//MaxChunk is the highest bit of a chunk, 16 for base32 and 32 for base64
char *baseXXtobinary(char *RetStr, const char *In, const char *Encoder, int MaxChunk)
{
    const char *ptr, *found;
    uint32_t val=0, bit;

    for (ptr=In; (*ptr != '\0') ; ptr++)
    {
//find the position in the base32 characterset. That position is the 'value' of the character
        found=strchr(Encoder, *ptr);
        if (found) val=(found - Encoder);
        else break;

        bit=MaxChunk;

//unpack the value into a string of 1's and 0's. As we add 1's and 0's from later characters
//we will get a contiguous stream of 1's and 0's that we can then break up into 8-bit chunks
        while (bit > 0)
        {
            if (val & bit) RetStr=CatStr(RetStr, "1");
            else RetStr=CatStr(RetStr, "0");
            bit=bit >> 1;
        }
    }

    return(RetStr);
}


int baseXXdecode(unsigned char *Out, const char *In, const char *Encoder, int MaxChunk)
{
    char *Tempstr=NULL;
    unsigned char *p_Out;
    const char *ptr, *end;
    uint32_t val;

    Tempstr=baseXXtobinary(Tempstr, In, Encoder, MaxChunk);

    val=StrLen(Tempstr);
    val -= val % 8;
    StrUnsafeTrunc(Tempstr, val);
    end=Tempstr+val;

    p_Out=Out;

    for (ptr=Tempstr; ptr < end; ptr+=8)
    {
        val=parse_bcd_byte(ptr);
        *p_Out=val & 0xFF;
        p_Out++;
    }
    *p_Out='\0';

    val=p_Out - Out;

    Destroy(Tempstr);

    return(val);
}




char *baseXXencode(char *RetStr, const char *Input, int Len, int ChunkSize, const char *Encoder, char Pad)
{
    char *Tempstr=NULL, *BCD=NULL;
    const char *ptr, *end;
    int val, len=0;


    RetStr=CopyStr(RetStr, "");
    BCD=encode_bcd_bytes(BCD, (unsigned const char *) Input, Len);
    end=BCD+StrLen(BCD);

    for (ptr=BCD; ptr < end; ptr+=ChunkSize)
    {
        Tempstr=CopyStrLen(Tempstr, ptr, ChunkSize);
        Tempstr=PadStrTo(Tempstr, '0', ChunkSize);
        val=(int) parse_bcd_byte(Tempstr);
        RetStr=AddCharToBuffer(RetStr, len, Encoder[val]);
        len++;
    }

    val=StrLen(RetStr) % 8;
    if (val > 0)
    {
        for (; val < 8; val++) RetStr=AddCharToStr(RetStr, Pad);
    }

    Destroy(Tempstr);
    Destroy(BCD);
    return(RetStr);
}



