
/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: GPL-3.0
*/

#ifndef LIBUSEFUL_STRINGLIST_H
#define LIBUSEFUL_STRINGLIST_H

/*
Utility functions to hand a string of strings, separated by a separator string or character
*/


#ifdef __cplusplus
extern "C" {
#endif

int InStringList(const char *Item, const char *List, const char *Sep);


#ifdef __cplusplus
}
#endif

#endif
