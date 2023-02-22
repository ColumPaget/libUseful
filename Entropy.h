/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: GPL-3.0
*/

#ifndef LIBUSEFUL_ENTROPY_H
#define LIBUSEFUL_ENTROPY_H

#include "defines.h"
#include "includes.h"


#ifdef __cplusplus
extern "C" {
#endif

//Creates a bunch of random bytes using /dev/urandom if available, otherwise falling back to weaker methods
int GenerateRandomBytes(char **RetBuff, int ReqLen, int Encoding);

//get a hexidecimamlly encoded random string
char *GetRandomHexStr(char *RetBuff, int len);

//get a random string containing only the characters in 'AllowedChars'
char *GetRandomData(char *RetBuff, int len, char *AllowedChars);

//get a random string of alphanumeric characters
char *GetRandomAlphabetStr(char *RetBuff, int len);


#ifdef __cplusplus
}
#endif


#endif


