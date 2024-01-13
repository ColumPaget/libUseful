#ifndef LIBUSEFUL_BASE32_H
#define LIBUSEFUL_BASE32_H

#include "includes.h"

int base32decode(unsigned char *Out, const char *In, const char *Encoder);
char *base32encode(char *RetStr, const char *Input, int Len, const char *Encoder, char Pad);

#endif
