#include "WebSocket.h"
#include "includes.h"
#include "Encodings.h"
#include "Entropy.h"
#include "Http.h"

//WebSockets. What an effin abortion. This is what a protocol looks like when designed by a committee of people who are either malicious, or very stupid
//Data is sent as frames, so you can interleave 'ping' commands in the middle of a data stream. I'm sure that gets a lot of use.
//Then, instead of having a dedicated 'flags' byte, flags are mixed in as the top bits of the operation and length bytes.
//This saves an entire 1 byte per frame, and a frame can apparently be Gigabytes in size if you want (which rather defeats the whole point of frames).
//Oh, but on top of that if the length of data is under 125 bytes, then it's sent as one byte, else it's sent as 3 bytes, and if greater than can
//be expressed in a uint16, it's sent as nine bytes!
//This is bad design. You shouldn't try to 'save' bits and bytes by splitting bytes between operations, this isn't the 1960s anymore, we can spare a byte.
//Pick a sensible maximum frame size, and dedicate bytes to it, instead of coming up with ad-hoc length encoding schemes.
//And 64-bits is not a sensible maximum. This will result in implementations where a malicious server can DOS a client by sending gigabytes in a single frame
//and a malicious client can to the same to a server.
//if you've already complicated your protocol with frames, what's the point of letting servers and clients send their entire memory space in one frame?


#define WS_FIN 128
#define WS_OP_MASK 15 //currently values below 16 for operations
#define WS_TEXT   0x1
#define WS_BINARY 0x2
#define WS_CLOSE  0x8
#define WS_PING   0x9
#define WS_PONG   0xA
#define WS_MASKED 128







static int WebSocketBasicHeader(char *Buffer, int op, int len, uint32_t mask)
{
    Buffer[0]=WS_FIN | op;
    Buffer[1]=WS_MASKED | len;
    memcpy(Buffer+2, &mask, 4);
    return(6);
}


static uint32_t WebSocketExtendedHeader(char *Buffer, int op, int len, uint32_t mask)
{
    int nlen;


    Buffer[0]=WS_FIN | op;
    Buffer[1]=WS_MASKED | 126;
    nlen=htons(len);
    memcpy(Buffer+2, &nlen, 2);
    memcpy(Buffer+4, &mask, 4);

    return(8);
}

static uint32_t WebSocketHeader(char *Buffer, int op, int len, uint32_t mask)
{
    if (len > 125) return(WebSocketExtendedHeader(Buffer, op, len, mask));
    else return(WebSocketBasicHeader(Buffer, op, len, mask));
}


//this both masks and unmasks masked data. The mask is 4 random characters that are XORed with the data
static void WebSocketMaskData(char *Data, const char *Mask, int len)
{
    int i, pos=0;

    for (i=0; i < len; i++)
    {
        Data[i]= Data[i] ^ Mask[pos];
        pos++;
        if (pos > 3) pos=0;
    }

}



//send control frames like PING, PONG and CLOSE
static void WebSocketSendControl(int Control, STREAM *S)
{
    char *Tempstr=NULL;
    int len;
    uint32_t mask;

    mask=rand() & 0xFFFFFFFF;
    Tempstr=SetStrLen(Tempstr, 20);
    len=WebSocketHeader(Tempstr, Control, 0, mask);
    STREAMPushBytes(S, Tempstr, len);
    Destroy(Tempstr);
}





static int WebSocketReadFrameHeader(STREAM *S)
{
    char bytes[2];
    uint16_t nlen;
    unsigned int len;
    int op, result;

    result=STREAMPullBytes(S, bytes, 2);
    if (result==0) return(0);

    if (bytes[0] & WS_FIN) S->State |= SS_MSG_READ;
    op=bytes[0] & WS_OP_MASK;

    len=bytes[1] & 0xFF;

    if (len == 127) printf("ERROR: BigLen\n");
    else if (len == 126)
    {
        STREAMPullBytes(S, (char *) &nlen, 2);
        len=ntohs(nlen);
    }

    switch (op)
    {
    case WS_CLOSE:
        return(STREAM_CLOSED);
        break;

    case WS_TEXT:
    case WS_BINARY:
        return(len);
        break;

    case WS_PING:
        WebSocketSendControl(WS_PONG, S);
        return(0);
        break;
    }

    return(0);
}


//send a frame of data
static void WebSocketSendFrame(STREAM *S, const char *Data, int Len)
{
    char *Frame=NULL;
    uint32_t mask;
    int pos;

    mask=rand() & 0xFFFFFFFF;
    if (mask==0) mask=12345;
    Frame=SetStrLen(Frame, Len + 20);
    if (S->Flags & SF_BINARY) pos=WebSocketHeader(Frame, WS_BINARY, Len, mask);
    else pos=WebSocketHeader(Frame, WS_TEXT, Len, mask);
    memcpy(Frame + pos, Data, Len);
    WebSocketMaskData(Frame + pos, (const char *) &mask, Len);
    STREAMPushBytes(S, Frame, pos + Len);
    Destroy(Frame);
}



int WebSocketSendBytes(STREAM *S, const char *Data, int Len)
{
    WebSocketSendFrame(S, Data, Len);
    return(Len);
}


int WebSocketReadBytes(STREAM *S, char *Data, int Len)
{
    static int msg_len=0;
    int read_len;
    int result=0;

    if (msg_len==0)
    {
        if (S->State & SS_MSG_READ)
        {
            if (Len > 0)
            {
                S->State &= ~ SS_MSG_READ;;
                Data[0]='\n';
                return(1);
            }
            return(0);
        }
        else
        {
            result=WebSocketReadFrameHeader(S);
            if (result > 0) msg_len=result;
        }
    }

    if (msg_len > 0)
    {
        if (msg_len > Len) read_len=Len;
        else read_len=msg_len;

        result=STREAMPullBytes(S, Data, read_len);
        if (result > 0) msg_len -= result;
    }

    return(result);
}







STREAM *WebSocketOpen(const char *WebsocketURL, const char *Config)
{
    STREAM *S;
    const char *p_Proto;
    char *URL=NULL, *Args=NULL, *Key=NULL, *Tempstr=NULL;
    int Port, Type;


    if (strncmp(WebsocketURL, "wss:", 4)==0)
    {
        URL=MCopyStr(URL, "https:", WebsocketURL + 4);
        Type=STREAM_TYPE_WSS;
    }
    else
    {
        URL=MCopyStr(URL, "http:", WebsocketURL + 3);
        Type=STREAM_TYPE_WS;
    }

    LibUsefulSetValue("HTTP:Debug", "Y");
    Tempstr=GetRandomAlphabetStr(Tempstr, 16);
    Key=EncodeBytes(Key, Tempstr, 16, ENCODE_BASE64);
    Args=MCopyStr(Args, Config, " Upgrade=websocket Connection=Upgrade Sec-Websocket-Key=", Key, " Sec-Websocket-Version=13", NULL);
    S=HTTPWithConfig(URL, Args);

    if (S)
    {
        S->Type=Type;
        WebSocketSendControl(WS_PING, S);
    }
    else
    {
        STREAMClose(S);
        S=NULL;
    }

    Destroy(Key);
    Destroy(URL);
    Destroy(Args);
    Destroy(Tempstr);


    return(S);
}
