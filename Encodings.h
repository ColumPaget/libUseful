/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef LIBUSEFUL_ENCODINGS_H
#define LIBUSEFUL_ENCODINGS_H

#include "includes.h"

/*
These functions encode and decode between different encodings.

'EncodingParse' is not important to C programmers, it's used when binding to languages that have poor
support for bit flags, and converts and encoding name to one of the #defined values below.

None of these encodings do things like add headers (as in uuencode and yencode) or splitting the output
into lines, they simply do the core encoding as one giant encoded string.


Please note that ASCII85 and Z85 are currently ENCODE ONLY. All other encodings can be used in either
encode or decode.

ENCODE_HEX causes the output of hex with lower-case 'abcdef', ENCODE_HEXUPPER uses 'ABCDEF'. Either
flag should decode either case via DecodeBytes.

ENCODE_IBASE64 and ENCODE_PBASE64 are just versions of BASE64 encoding with different character maps

ENCODE_UUENC, ENCODE_XXENC and ENCODE_CRYPT are variants of BASE64 used by the uuencode, xxencode
programs, and by the linux 'crypt' function.

EncodeBytes expects to be passed a pointer to memory allocated with malloc or calloc. It will
realloc this memory to be larger if it needs more room. You can pass it NULL and it will allocate
fresh memory. It returns a pointer to the resulting memory holding its output. This memory will have
to be freed with 'DestroyString' or 'free'. e.g.:



char *Encoded=NULL;

Encoded=EncodeBytes(Encoded, "my bytes", 8, ENCODE_BASE64);
printf("ENCODED: %s\n",Encoded);

DestroyString(Encoded);



DecodeBytes needs to return the length of data decoded, as it could be binary data and so we can't
just treat it as a null-terminated string. Thus DecodeBytes is passed a char ** which it will
resize if it needs to


char *Decoded=NULL;
int outlen;

outlen=DecodeBytes(&Decoded, InputBytes, InputLen, ENCODE_BASE64);



If you *KNOW* that your output of DecodeBytes is going to be null-terminated text then you can use
'DecodeToText' which works exactly like EncodeBytes
*/

#define ENCODE_NONE        0
#define ENCODE_QUOTED_MIME 1
#define ENCODE_BINARY      2
#define ENCODE_YENCODE     3
#define ENCODE_OCTAL       8
#define ENCODE_DECIMAL    10
#define ENCODE_HEX        16
#define ENCODE_HEXUPPER   17
#define ENCODE_BASE32  32  //rfc4648 base 32
#define ENCODE_CBASE32 33  //crockford base32
#define ENCODE_HBASE32 34  //'extended hex' base32
#define ENCODE_BASE32_HEX 34
#define ENCODE_WBASE32 35  //'word safe' base32
#define ENCODE_BASE32_WORDSAFE 35
#define ENCODE_ZBASE32 36
#define ENCODE_BECH32  37
#define ENCODE_BASE64  64
#define ENCODE_IBASE64 65
#define ENCODE_PBASE64 66
#define ENCODE_XXENC 67
#define ENCODE_UUENC 68
#define ENCODE_CRYPT 69
#define ENCODE_RBASE64 70

#define ENCODE_ASCII85 85
#define ENCODE_Z85 86
#define ENCODE_QUOTED_HTTP 404

#define HEX_CHARS "0123456789ABCDEF"
#define ALPHA_CHARS "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define BASE64_CHARS  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define RBASE64_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" //RFC4648 compliant
#define IBASE64_CHARS "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789+/"
#define PBASE64_CHARS "0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
#define CRYPT_CHARS "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define UUENC_CHARS "`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
#define XXENC_CHARS "+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define ASCII85_CHARS "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu"
#define Z85_CHARS "01234567899abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#."

#define BASE32_CROCKFORD_CHARS "0123456789abcdefghjkmnpqrstvwxyz"
#define BASE32_RFC4648_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"
#define BASE32_HEX_CHARS "0123456789ABCDEFGHIJKLMNOPQRSTUV"
#define BASE32_WORDSAFE_CHARS "23456789CFGHJMPQRVWXcfghjmpqrvwx"
#define BASE32_ZBASE32_CHARS "ybndrfg8ejkmcpqxot1uwisza345h769"
#define BASE32_BECH32_CHARS "qpzry9x8gf2tvdw0s3jn54khce6mua7l"


#ifdef __cplusplus
extern "C" {
#endif

//parse a string that describes an encoding. Strings are:
//"8"    Octal Encoding
//"oct"  Octal Encoding
//"10"   Decimal Encoding
//"dec"  Decimal Encoding
//"16"   HexaDecimal Encoding
//"hex"  HexaDecimal Encoding
//"32"   base32 Encoding
//"b32"  base32 Encoding
//"64"   base64 Encoding
//"b64"  base64 Encoding
//"r64"  rfc4648 compliant alternative base64 Encoding
//"rfc4648"  rfc4648 compliant alternative base64 Encoding
//"i64"  alternative base64 Encoding
//"p64"  another alternative base64 Encoding
//"xx"   xxencode encoding
//"uu"   uuencode encoding
//"crypt"   unix 'crypt' encoding
//"ascii85" ascii85 encoding
//"z86" z85 encoding


int EncodingParse(const char *Str);
char *EncodeBytes(char *Buffer, const char *Bytes, int len, int Encoding);
int DecodeBytes(char **Return, const char *Text, int Encoding);
char *DecodeToText(char *RetStr, const char *Text, int Encoding);


#ifdef __cplusplus
}
#endif

#endif
