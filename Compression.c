#include "Compression.h"
#include "DataProcessing.h"

/*
This module mostly contains functions for compressing/decompressing using zlib.
zlib is the compression method used by gzip and zip/pkzip. This allows libUseful
to unzip compressed http pages and files compressed with gzip.

This module also contains functions for auto-detecting type of compression used on
a file, if it is one of gzip, bzip2 or lzip
*/


//auto-detect compression type from first bytes in the stream
//this function assumes it is passed an open stream that is
//positioned at the start of file or stream
const char *STREAMDetectCompression(STREAM *S)
{
    char *Buffer=NULL;
    const char *Comp="";
    int result;

    if (S->Type==STREAM_TYPE_FILE)
    {
        Buffer=SetStrLen(Buffer, 20);
        //using PeekBytes is useless here, because Peek still reads from the file
        //at the time that the compression Processor isn't loaded into the stream.
        //This means bytes will be read into the stream buffer, and sit there, which
        //haven't been decompressed. Thus we have to SEEK back, and flush out the stream
        //after we have examined the leading bytes.
        result=STREAMReadBytes(S, Buffer, 10);
        STREAMSeek(S, 0, SEEK_SET);


        if ((result > 2) && (strncmp(Buffer, "BZh", 3)==0)) Comp="bzip2";
        else if ((result > 3) && (strncmp(Buffer, "LZIP", 4)==0)) Comp="lzip";
        else if ((result > 1) && (strncmp(Buffer, "\x1F\x8B", 2)==0)) Comp="gzip";
        else if ((result > 5) && (strncmp(Buffer, "\xFD\x37\x7A\x58\x5A\x00", 6)==0)) Comp="xz";
    }

    Destroy(Buffer);

    return(Comp);
}



// Rather than writing a compressed stream, compress a bunch of bytes in a buffer
int CompressBytes(char **Out, const char *Alg, const char *In, unsigned long Len, int Level)
{
    TProcessingModule *Mod=NULL;
    char *Tempstr=NULL;
    unsigned long val;
    int result;

    Tempstr=FormatStr(Tempstr,"CompressionLevel=%d",Level);
    Mod=StandardDataProcessorCreate("compress",Alg,Tempstr, NULL, NULL);
    if (! Mod) return(-1);

    val=Len *2;
    *Out=(char *) realloc(*Out,val);
    result=Mod->Write(Mod,In,Len,Out,&val,TRUE);

    DestroyString(Tempstr);
    DataProcessorDestroy(Mod);

    return(result);
}


//rather than reading a compressed stream, decompress a bunch of bytes in a buffer
int DeCompressBytes(char **Out, const char *Alg, const char *In, unsigned long Len)
{
    TProcessingModule *Mod=NULL;
    int result;
    unsigned long val;

    Mod=StandardDataProcessorCreate("decompress",Alg,"",NULL, NULL);
    if (! Mod) return(-1);

    val=Len *2;
    *Out=(char *) realloc(*Out,val);
    result=Mod->Read(Mod,In,Len,Out,&val,TRUE);

    DataProcessorDestroy(Mod);

    return(result);
}




// create a compression module, this will be a Data Processing Module that either
// uses libraries (currently just zlib) to compress and decompress or
// uses compression commands like gzip, bzip2 and xz via a pipe
TProcessingModule *LU_CompressionModuleCreate(const char *Name, const char *Args)
{
    TProcessingModule *Mod=NULL;

    if (
        (strcasecmp(Name,"zlib")==0)  ||
        (strcasecmp(Name,"deflate")==0)
    )
    {
#ifdef HAVE_LIBZ
        Mod=DataProcessorCreate(Name, Args);
        Mod->Init=zlibProcessorInit;
        Mod->Flags |= DPM_COMPRESS;
#endif
    }
    else if (
        (strcasecmp(Name,"gzip")==0) ||
        (strcasecmp(Name,"gz")==0)
    )
    {
#ifdef HAVE_LIBZ
        Mod=DataProcessorCreate(Name, Args);
        Mod->Init=zlibProcessorInit;
        Mod->Args=MCopyStr(Mod->Args,"Alg=gzip ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;

#else
        Mod=PipeCommandProcessorCreate(Name, Args);
        Mod->Args=MCopyStr(Mod->Args,"Command='gzip --stdout -' ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;
#endif
    }
    else if (
        (strcasecmp(Name,"bzip2")==0) ||
        (strcasecmp(Name,"bz2")==0)
    )
    {
        Mod=PipeCommandProcessorCreate(Name, Args);
        Mod->Args=MCopyStr(Mod->Args,"Command='bzip2 --stdout -' ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;
    }
    else if (strcasecmp(Name,"xz")==0)
    {
        Mod=PipeCommandProcessorCreate(Name, Args);
        Mod->Args=MCopyStr(Mod->Args,"Command='xz -z --stdout -' ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;
    }
    else if (strcasecmp(Name,"lzip")==0)
    {
        Mod=PipeCommandProcessorCreate(Name, Args);
        Mod->Args=MCopyStr(Mod->Args,"Command='lzip --stdout -' ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;
    }



    return(Mod);
}


// create a decompression module, this will be a Data Processing Module that either
// uses libraries (currently just zlib) to compress and decompress or
// uses compression commands like gzip, bzip2 and xz via a pipe
TProcessingModule *LU_DeCompressionModuleCreate(const char *Name, const char *Args)
{
    TProcessingModule *Mod=NULL;

    if (strcasecmp(Name,"zlib")==0)
    {
#ifdef HAVE_LIBZ
        Mod=DataProcessorCreate(Name, Args);
        Mod->Init=zlibProcessorInit;
        Mod->Flags |= DPM_COMPRESS;
#endif
    }
    else if (strcasecmp(Name,"gzip")==0)
    {
#ifdef HAVE_LIBZ
        Mod=DataProcessorCreate(Name, Args);
        Mod->Init=zlibProcessorInit;
        Mod->Args=MCopyStr(Mod->Args,"Alg=gzip ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;

#else
        Mod=PipeCommandProcessorCreate(Name, Args);
        Mod->Args=MCopyStr(Mod->Args,"Command='bzip2 -d --stdout -' ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;
#endif
    }
    else if (strcasecmp(Name,"bzip2")==0)
    {
        Mod=PipeCommandProcessorCreate(Name, Args);
        Mod->Args=MCopyStr(Mod->Args,"Command='bzip2 -d --stdout -' ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;
    }
    else if (strcasecmp(Name,"xz")==0)
    {
        Mod=PipeCommandProcessorCreate(Name, Args);
        Mod->Args=MCopyStr(Mod->Args,"Command='xz -d --stdout -' ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;
    }
    else if (strcasecmp(Name,"lzip")==0)
    {
        Mod=PipeCommandProcessorCreate(Name, Args);
        Mod->Args=MCopyStr(Mod->Args,"Command='lzip -d --stdout -' ", Args, NULL);
        Mod->Flags |= DPM_COMPRESS;
    }



    return(Mod);
}




/* From here on down is all zlib data processor */


#ifdef HAVE_LIBZ

#include <zlib.h>

typedef struct
{
    z_stream z_in;
    z_stream z_out;
} zlibData;
#endif



//Zlib is a little weird. It accepts a pointer to a buffer (next_in) and a buffer length (avail_in) to specify the input
//and another buffer (next_out) and length (avail_out) to write data into. When called it reads bytes from next_in, updates
//next_in to point to the end of what it read, and subtracts the number of bytes it read from avail_in so that avail_in
//now says how many UNUSED bytes there are pointed to by next_in. Similarly it writes to next_out, updating that pointer
//to point to the end of the write, and updating avail_out to say how much room is LEFT usused in the output buffer
//
//However, if zlib doesn't use all avail_in, then you can't mess with that buffer until it has. Hence you can't take the unusued
//data from next_in/avail_in and copy it to a new buffer and pass that buffer into deflate/inflate on the next call. If zlib
//doesn't use all the input the only way to handle it is to grow the output buffer and call inflate/deflate again, so that it
//can write into the expanded buffer until it's used up all input.
//
//Finally, when you've supplied all the input you've got, you have to call deflate with 'Z_FINISH' so that it knows there's no
//more data coming.

int zlibProcessorWrite(TProcessingModule *ProcMod, const char *InData, unsigned long InLen, char **OutData, unsigned long *OutLen, int Flush)
{
    int bytes_wrote=0;
#ifdef HAVE_LIBZ
    int val=0;
    zlibData *ZData;

    if (ProcMod->Flags & DPM_WRITE_FINAL) return(STREAM_CLOSED);
    ZData=(zlibData *) ProcMod->Data;


    ZData->z_out.avail_in=InLen;
    ZData->z_out.next_in=(Bytef *) InData;
    ZData->z_out.avail_out=*OutLen;
    ZData->z_out.next_out=(Bytef *) *OutData;

    while ((ZData->z_out.avail_in > 0) || Flush)
    {
        if (Flush) val=deflate(& ZData->z_out, Z_FINISH);
        else val=deflate(& ZData->z_out, Z_NO_FLUSH);

        bytes_wrote=*OutLen-ZData->z_out.avail_out;
        if (val==Z_STREAM_END)
        {
            ProcMod->Flags |= DPM_WRITE_FINAL;
            break;
        }

        if ((ZData->z_out.avail_in > 0) || Flush)
        {
            *OutLen+=BUFSIZ;
            *OutData=(char *) realloc(*OutData,*OutLen);
            ZData->z_out.avail_out+=BUFSIZ;
        }

    }



#endif
    return(bytes_wrote);
}


int zlibProcessorRead(TProcessingModule *ProcMod, const char *InData, unsigned long InLen, char **OutData, unsigned long *OutLen, int Flush)
{
    int bytes_read=0;

#ifdef HAVE_LIBZ
    int result=0;
    zlibData *ZData;

    if (ProcMod->Flags & DPM_READ_FINAL)
    {
        return(STREAM_CLOSED);
    }

    ZData=(zlibData *) ProcMod->Data;
    ZData->z_in.avail_in=InLen;
    ZData->z_in.next_in=(Bytef *) InData;
    ZData->z_in.avail_out=*OutLen;
    ZData->z_in.next_out=(Bytef *) *OutData;

    while ((ZData->z_in.avail_in > 0) || Flush)
    {
        //We do not need Z_FINISH here,
        result=inflate(& ZData->z_in, Z_NO_FLUSH);
        bytes_read=(*OutLen)-ZData->z_in.avail_out;


        if (result==Z_BUF_ERROR) break;
        switch (result)
        {
        case Z_DATA_ERROR:
            inflateSync(&ZData->z_in);
            break;

        case Z_ERRNO:
            if (Flush) ProcMod->Flags |= DPM_READ_FINAL;
            break;

        case Z_STREAM_ERROR:
        case Z_STREAM_END:
            ProcMod->Flags |= DPM_READ_FINAL;
            break;
        }

        if ((ZData->z_in.avail_in==0) && (ProcMod->Flags & DPM_READ_FINAL)) break;

        if ((ZData->z_in.avail_in > 0) || Flush)
        {
            (*OutLen)+=BUFSIZ;
            *OutData=(char *) realloc(*OutData,*OutLen);
            ZData->z_in.next_out=(Bytef *) (*OutData) + bytes_read;
            ZData->z_in.avail_out=(*OutLen) - bytes_read;
        }
    }
#endif

    if ((bytes_read==0) && Flush) return(STREAM_CLOSED);
    return(bytes_read);
}




int zlibProcessorClose(TProcessingModule *ProcMod)
{
#ifdef HAVE_LIBZ
    zlibData *ZData;

    ZData=(zlibData *) ProcMod->Data;
    if (ZData)
    {
        inflateEnd(&ZData->z_in);
        deflateEnd(&ZData->z_out);

        free(ZData);
        ProcMod->Data=NULL;
    }
#endif
    return(TRUE);
}


#define COMP_ZLIB 0
#define COMP_GZIP 1

int zlibProcessorInit(TProcessingModule *ProcMod, const char *Args, unsigned char **Header, int *HeadLen)
{
    int result=FALSE;

#ifdef HAVE_LIBZ
    zlibData *ZData;
    int CompressionLevel=5;
    char *Name=NULL, *Value=NULL;
    const char *ptr;
    int Type=COMP_ZLIB;

    ptr=GetNameValuePair(Args,"\\S","=",&Name,&Value);
    while (ptr)
    {
        if (strcasecmp(Name,"Alg")==0)
        {
            if (strcasecmp(Value, "gzip")==0) Type=COMP_GZIP;
        }
        else if (strcasecmp(Name,"CompressionLevel")==0) CompressionLevel=atoi(Value);
        else if (strcasecmp(Name,"Level")==0) CompressionLevel=atoi(Value);
        ptr=GetNameValuePair(ptr,"\\S","=",&Name,&Value);
    }


    ProcMod->ReadMax=4096;
    ProcMod->WriteMax=4096;


    ZData=(zlibData *) calloc(1,sizeof(zlibData));
    ZData->z_in.avail_in=0;
    ZData->z_in.avail_out=0;

    if (Type==COMP_GZIP) result=inflateInit2(&ZData->z_in, 47);
    else result=inflateInit(&ZData->z_in);

    if (result != Z_OK) RaiseError(0, "zlib init failed: %s", ZData->z_in.msg);

    ZData->z_out.avail_in=0;
    ZData->z_out.avail_out=0;
    if (Type==COMP_GZIP) deflateInit2(&ZData->z_out,5,Z_DEFLATED,30,8,Z_DEFAULT_STRATEGY);
    else deflateInit(&ZData->z_out,CompressionLevel);

    ProcMod->Data=(void *) ZData;
    result=TRUE;

    ProcMod->Read=zlibProcessorRead;
    ProcMod->Write=zlibProcessorWrite;
    ProcMod->Close=zlibProcessorClose;

    DestroyString(Name);
    DestroyString(Value);

#endif
    return(result);
}




