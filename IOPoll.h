/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/


#ifndef LIBUSEFUL_IOPOLL_H
#define LIBUSEFUL_IOPOLL_H

#include "includes.h"
#include "Stream.h"



//These flage are used to tell FDSelect whether to watch a stream for read, write, or both
#define SELECT_READ 1
#define SELECT_WRITE 2


#ifdef __cplusplus
extern "C" {
#endif


//watch a file descriptor for activity. 'Flags' can be SELECT_READ, and/or SELECT_WRITE depending on what is being watched for
int FDSelect(int fd, int Flags, struct timeval *tv);

//is file ready to recieve bytes?
int FDIsWritable(int fd);

//are bytes available to be read? 
int FDCheckForBytes(int fd);


//use this macro to check if a non-blocking stream is ready for write.
//For reading use STREAMCheckForBytes
#define STREAMIsWriteable(S) (FDIsWritable((S)->out_fd))


//Not usually called by the application programmer, this is called by ListDestroy
//in order to remove it from our internal list of STREAMSelect lists, if it's in there
void STREAMSelectUnregister(ListNode *Select);


//Not usually called by the application programmer, this is called by STREAMClose
//in order to remove a 'dead' stream from our STREAMSelectLists
void STREAMSelectsRemoveStream(STREAM *S);



// watch a list of streams for READ, return the first one that has activity.
// this function performs like 'select', however if uses either 'poll' or 'select'
// depending on which of those to the OS supports. Currently it only supports
// watching streams for input that can be read.

// Like 'select' the 'tv' value allows setting a timeout in seconds and micro-seconds
// after which STREAMSelect will return NULL, indicating that it timed out before
// any stream had activity. If a stream is active, and is returned, before the timeout
// expires 'tv' will contain the remaining time in seconds and microseconds. Thus
// 'STREAMSelect' can be used to drive periodic actions in combination with watching
// lists of file descriptors or sockets.

// STREAMSelect has a feature where, when a stream has been active, it is moved
// to the bottom of the list so that constant activity on that stream does not
// lock out other streams. 

// If 'epoll' is supported by the OS, and libUseful was compiled with 'epoll' support
// STREAMSelect will use 'epoll' if the list is greater than 5 items

// If using 'select' STREAMSelect will refuse to add Streams when the list is 
// longer than FD_SETSIZE. However, it will move Streams not added to the
// start of the 'Streams' list so they get their turn next time. Used in combination
// with a short 'tv' timeval, this mitigates large select lists on platforms that
// only suppport 'select'

STREAM *STREAMSelect(ListNode *Streams, struct timeval *tv);

#ifdef __cplusplus
}
#endif


#endif

