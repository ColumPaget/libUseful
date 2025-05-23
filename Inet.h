/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef LIBUSEFUL_INET_H
#define LIBUSEFUL_INET_H

#include "includes.h"
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

char *ExtractFromWebpage(char *RetStr, const char *URL, const char *ExtractStr, int MinLength);
char *GetExternalIP(char *RetStr);
int IPGeoLocate(const char *IP, ListNode *Vars);

#ifdef __cplusplus
}
#endif


#endif
