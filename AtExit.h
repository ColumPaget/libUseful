/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef LIBUSEFUL_AT_EXIT_H
#define LIBUSEFUL_AT_EXIT_H

#ifdef __cplusplus
extern "C" {
#endif

// Everthing in here relates to functions called at program exit to do certain cleaning up. 
// It's of no concern of the user.
// Nothing to see here, move along


//this function will be called at program exit
void LibUsefulAtExit();

//this sets up the 'at exit' feature
void LibUsefulSetupAtExit();

#ifdef __cplusplus
};
#endif


#endif
