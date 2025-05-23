/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef LIBUSEFUL_STREAM_H
#define LIBUSEFUL_STREAM_H

/*

The STREAM object and its functions is the main means of accessing files and network resources in libUseful. 

STREAMOpen returns a stream object and takes two arguments, a URL and a 'Config' argument

//for example

S=STREAMOpen("/tmp/myfile.txt", "w");

The first argument can be any of the following types:

/tmp/myfile.txt                          file
file:///tmp/myfile.txt                   file, web-browser style. Note 3 '/' symbols. 
mmap:/tmp/myfile.txt                     memory mapped file
tty:/dev/ttyS0:38400                     open a serial device, in this case at 38400 baud
udp:192.168.2.1:53                       udp socket (Use STREAMSendDgram, from Socket.h to send packets)
tcp:192.168.2.1:25                       tcp network connection
ssl:192.168.2.1:443                      tcp network connection with encryption
tls:192.168.2.1:443                      tcp network connection with encryption
bcast:255.255.255.255:5353               udp broadcast socket (Use STREAMSendDgram, from Socket.h to send packets)
ws:nos.lol/                              websocket
wss:nos.lol/                             secure websocket
unix:/tmp/socket                         unix socket
unixdgram:/tmp/socket                    unix datagram socket
http:user:password@www.google.com        http network connection
https:www.google.com                     https network connection
cmd:cat /etc/hosts                       run command 'cat /etc/hosts' and read/write to/from it
ssh:192.168.2.1:1022/cat /etc/hosts      ssh connect, running the command 'cat /etc/hosts'
stdin:                                   standard in
stdout:                                  standard out
stdio:                                   both standard in and standard out

'file://' is provided for compatiblity with web-browser environments. In this url format the protocol part is 'file://'. If a third '/' is present, like so 'file:///etc/services' then the url is a full path from the filesystem root. Any lesser number of '/' indicates a relative path from the current directory

in the case of SSH stream the default action, if no 'config' flags are passed, is to run a command. 'x' config flag will also explictly run a command. 'r' will cat a file from the remote server. 'w' will cat from the stream TO a file on the remote server. 

S=STREAMOpen("ssh:192.168.2.1/myfile.txt", "r");  //read from 'myfile.txt' in current directory

S=STREAMOpen("ssh:192.168.2.1//tmp/myfile.txt", "r");  //read from 'myfile.txt' in /tmp (note double '/' for '/tmp')

S=STREAMOpen("ssh:192.168.2.1/myfile.txt", "w");  //WRITE to 'myfile.txt' in current directory. THIS ONLY WORKS FOR ASCII TEXT, NOT BINARY FILES.
S=STREAMOpen("ssh:192.168.2.1/myfile.txt", "a");  //APPEND to 'myfile.txt' in current directory. THIS ONLY WORKS FOR ASCII TEXT, NOT BINARY FILES.

"r", "w" and "a" modes use the 'cat' command on the remote server, so obviously that must exist


S=STREAMOpen("ssh:192.168.2.1/ls *", "");  //RUN COMMAND 'ls *'
S=STREAMOpen("ssh:192.168.2.1/ls *", "x");  //RUN COMMAND 'ls *'



The 'config' argument has different meanings for some of the different URL types.

For files and http the config argument is a string of characters each of which represents an option 
that modifies stream behavior, as with 'fopen'

c     create file
r     read only
w     write only
a     append 
+     make read-only, append or write-only be read-write
E     raise an error if this file fails to open
f  - 'Full Flush'. For file-based STREAMS whenever 'STREAMFlush' is called, call 'fsync' to force data write to disk.
F     follow symlinks. Without this flag an error is raised when a symlink is opened.
l     lock/unlock file on each read
L     lock/unlock file on each write
i     allow this file to be inherited across an exec (default is close-on-exec)
t     make a unique temporary file name. the file path must be a mktemp style template, with the last six characters being 'XXXXXX'
S     file contents are sorted
x     exclusive open using O_EXCL. Only create/open file if it doesn't exist.
z     compress/uncompress with gzip (uncompress autodetects gzip,bzip2 or no compression)
e     encrypt using openssl compatible file format
R     autorecovery. Take a backup when the file is opened for write, and if the file isn't closed properly, then revert to that backup when next it's opened.


for encrypted files with the 'e' option, a password must be supplied using the 'encrypt' argument. The key and inputvector are calculated from this password.
e.g.

S=STREAMOpen("myfile.enc", "we encrypt=T0PSekrit");

The default cipher used is aes-256-cbc, however the cipher can be overridden like so:


S=STREAMOpen("myfile.enc", "we encrypt=T0PSekrit encrypt_cipher=blowfish");

Encryption only supports either read only or write only files, not read-write.



Autorecovery using the 'R' option will take a backup whenever the file is opened write-only, but not for read-write or append. This backup should be deleted when the file is closed. If the file is opened for read or append, and the backup exists, then the file will be moved to <path>.<date>.error and the backup will be imported in it's place.



for tcp/unix/udp network connections the 'config argument' defaults to 'rw' if blank. 
Otherwise it is made up of the following options

r  - 'read' mode (a non-op as all sockets are readable)
w  - 'write' mode (a non-op as all sockets are writeable)
n  - nonblocking socket
E  - report socket connection errors
k  - TURN OFF socket keep alives
B  - broadcast socket
f  - 'Full Flush'. For TCP based STREAMS, whenever 'STREAMFlush' is called,  turn Nagle's algorithm off and on again using setsockopt, forcing data to be sent to the network immediately.
F  - TCP Fastopen
R  - Don't route (equivalent to applying SOCKOPT_DONTROUTE)
N  - TCP no-delay (disable Nagle algo)

This argument can be followed by name-value pairs such as:

ttl=<seconds>       set ttl of socket
tos=<value>         set tos of socket
mark=<value>        set SOCKOPT_MARK if supported
keepalive=<y/n>     turn on/off socket keepalives
timeout=<centisecs> connect/read timeout for socket

The options 'N' and 'f' both turn off the TCP Nagle algorithm. However 'N' turns it off completely, whereas 'f' only briefly turns it off when 'STREAMFlush' is called, which causes any cached data to be immediately written to the network. The reason for doing this is to handle situations where rapid exchange of short messages is required (e.g. industrial automation use cases). By default TCP uses the Nagle algorithm to queue data, waiting a while for more data so that it can send as much data in one packet as efficiently as possible, but this not desirable if speed of communications is important. However, the 'N' option that completely turns off Nagle on a socket, has been seen to cause some issues with simple embedded device peers. Possibly these issues are because such peers do not do full TCP message reconstruction, and simply treat a packet as a complete message, and are thus unable to handle messages spread across more than one packet. If Nagle is turned off, the O.S. might decided to send partial data, resulting in issues with these devices. 'f' offers an alternative use case, where a program can write it's full message to the network, and then call 'STREAMFlush' to force all the data to be sent immediately. 

UDP sockets might not work quite as you'd expect: The socket binds to a random port at the local end, and expects to send to the supplied port, rather the same way tcp works. If you want to bind a UDP socket to a specific local port, and receive and send messages from that port from/to other hosts, then you need to use STREAMServerNew from Server.h


For SSH streams config argument is a string of characters each of which represents an option 
that modifies stream behavior, as with 'fopen'. By default SSH streams are treated as a 
stream to a file, much like http, but if the 'x' flag is used, the file name is instead
treated as a command to be run, with the contents of the stream being the output from and
input to that command.

c     create file
r     read only
w     write only
a     append 
+     make read-only, append or write-only be read-write
E     raise an error if this file fails to open
i     allow this file to be inherited across an exec (default is close-on-exec)
S     file contents are sorted
x     treat file path as a command to execute (currently on in ssh: streams) 
z     compress/uncompress with gzip


for 'http', 'https', 'ws' and 'wss' URLs the first argument is a character list (though only one character long) with the following values

r    GET method (default if no method specified)
w    POST method
W    PUT method
D    DELETE method
P    PATCH method
H    HEAD  method

If the method used is 'GET' and TCP_QUICKACK is available, it is automatically enabled as no data is sent from the client during GET


method=<method>  //override method (GET/POST etc) with any default value
oauth=<oauth config to use>
content-type=<content type>         //content type of sent data (for PUT/POST)
content-length=<content length>     //content length of sent data (for PUT/POST)
user=<username>
useragent=<user agent>
user-agent=<user agent>
timeout=<centisecs>    //socket timeout in centisecs, this applies both to connection timeout and read timeout. If a different value is desired for read, set it with 'STREAMSetTimeout'
hostauth   //send auth details without waiting for 401 from server

Note, 'hostauth' is not a name/value pair, just a config flag that enables sending authentication without waiting for a 401 Response from the server. This means that we can't know the authentication realm for the server, and so internally use the hostname as the realm for looking up logon credentials. This is mostly useful for the github api.

custom headers can be added for http/https/ws/wss by adding an argument 'header=value', for example, a header named 'apikey':

S=STREAMOpen("https://myserver.com", "r apikey=4lkdafee323fxx");



For 'cmd' type URLs the config options are those detailed in "SpawnPrograms.h"
For 'tty' type URLs the config options are those detailed in "Pty.h" for "TTYConfigOpen"


*/



//the 'Type' variable in the STREAM object is set to one of thse values and is used internally for knowing how to handle a given stream
typedef enum {STREAM_TYPE_FILE, STREAM_TYPE_PIPE, STREAM_TYPE_TTY, STREAM_TYPE_UNIX, STREAM_TYPE_UNIX_DGRAM, STREAM_TYPE_TCP, STREAM_TYPE_UDP, STREAM_TYPE_SSL, STREAM_TYPE_HTTP, STREAM_TYPE_CHUNKED_HTTP, STREAM_TYPE_MESSAGEBUS, STREAM_TYPE_UNIX_SERVER, STREAM_TYPE_TCP_SERVER, STREAM_TYPE_UNIX_ACCEPT, STREAM_TYPE_TCP_ACCEPT, STREAM_TYPE_TPROXY, STREAM_TYPE_UPROXY, STREAM_TYPE_SSH, STREAM_TYPE_WS, STREAM_TYPE_WSS, STREAM_TYPE_HTTP_SERVER, STREAM_TYPE_HTTP_ACCEPT, STREAM_TYPE_WS_SERVER, STREAM_TYPE_WS_ACCEPT, STREAM_TYPE_WS_SERVICE} ESTREAMType;


#define STREAM_TYPE_TLS STREAM_TYPE_SSL


#include <fcntl.h>
#include <sys/time.h> //for 'struct timeval'
#include <stdint.h>
#include "List.h"
#include "Errors.h"




//error condition return values from functions like "STREAMReadBytes". Mostly you will just see 'STREAM_TIMEOUT', meaning no data was read in the 
//time that a stream it configured to block for, or STREAM_CLOSED, meaning the end-of-file has been reached, or that the remote peer has closed
//a network connection
#define STREAM_BYTES_READ 1
#define STREAM_TIMEOUT 0
#define STREAM_CLOSED -1
#define STREAM_NODATA -2
#define STREAM_MESSAGE_END -3
#define STREAM_DATA_ERROR -4


//Flags that alter stream flushing. These are set with 'STREAMSetFlushType' or with 'STREAMFileOpen'
#define FLUSH_FULL 0      //Flush when buffers are full
#define FLUSH_LINE 1      //Flush when a newline is written to the stream
#define FLUSH_BLOCK 2     //Flush in blocks (Blocksize specified in STREAMSetFlushType)
#define FLUSH_ALWAYS 4    //Flush on any write to a stream
#define FLUSH_BUFFER 8


//Flags that alter stream behavior, the first block can be passed to 'STREAMFileOpen' only
//These days it's better to use 'STREAMOpen' with a character string that defines stream options, rather
//than STREAMFileOpen and these flags. These flags are used internally though.

#define SF_RDWR 0 //open stream for read and write. This is the default

//FLUSH_ flags go in this gap

#define SF_RDONLY               16  //open stream read only 
#define SF_WRONLY               32  //open stream write only
#define SF_CREAT                64  //create stream if it doesn't exist
#define SF_CREATE               64  //create stream if it doesn't exist
#define STREAM_APPEND          128  //append to file
#define SF_TRUNC               256  //truncate file to zero bytes on open
#define SF_MMAP                512  //create a memory mapped file
#define SF_WRLOCK             1024  //ONLY FOR FILES: lock file on every write
#define SF_RDLOCK             2048  //lock file on every read
#define SF_FOLLOW             4096  //ONLY FOR FILES: follow symbolic links
#define SF_TLS                4096  //ONLY FOR SOCKETS: use SSL/TLS
#define SF_SECURE             8192  //lock internal buffers into memory so they aren't written to swap or coredumps
#define SF_NONBLOCK          16384  //nonblocking open (you must use select to check that the file is ready to use)
#define SF_EXCL              32768  //ONLY FOR FILES: exclusive create with O_EXCL, file must not pre-exist
#define SF_TLS_AUTO          32768  //nothing to see here, move along
#define SF_ERROR             65536  //raise an error if open or connect fails
#define SF_EXEC_INHERIT     131072  //allow stream to be inherited across an exec (default is close-on-exec)
#define SF_AUTORECOVER      262144  //ONLY FOR FILES: take autorecovery backup on writing a file, and apply it on read
#define SF_BINARY           262144  //ONLY FOR SOCKETS: 'binary mode' for, websockets etc
#define SF_NOCACHE          524288  //ONLY FOR FILES: don't cache file data in filesystem cache
#define SF_QUICKACK         524288  //ONLY FOR SOCKETS: set TCP_QUICKACK after every read
#define SF_LIST             524288  //only for SSH streams: list files
#define SF_SORTED          1048576  //file is sorted, this is a hint to 'STREAMFind'
#define STREAM_IMMUTABLE   2097152  //file is immutable (if supported by fs)
#define STREAM_APPENDONLY  4194304  //file is append-only (if supported by fs)
#define SF_COMPRESSED      8388608  //enable compression, this requests compression
#define SF_TMPNAME        16777216  //file path is a template to create a temporary file name (must end in 'XXXXXX')
#define SF_ENCRYPT        33554432  //file path is a template to create a temporary file name (must end in 'XXXXXX')
#define SF_FULL_FLUSH     67108864  //On 'STREAMFlush(S) force a full flush of data, using fsync to write data to disk for files, or overriding TCP's Nagle algorithm for sockets


//Stream state values, set in S->State
#define LU_SS_CONNECTING 1
#define LU_SS_INITIAL_CONNECT_DONE 4
#define LU_SS_CONNECTED 8
#define LU_SS_DATA_ERROR 16
#define LU_SS_WRITE_ERROR 32
#define LU_SS_EMBARGOED 64
#define LU_SS_SSL  4096
#define LU_SS_AUTH 8192
#define LU_SS_COMPRESSED 16384 //compression enabled, specifies compression active on a stream. Since libUseful 5.43 reading a compressed file will autodetect gzip,bzip2 or no compression.
#define LU_SS_MSG_READ 32768

//state values available for programmer use
#define LU_SS_USER1 268435456
#define LU_SS_USER2 536870912
#define LU_SS_USER3 1073741824
#define LU_SS_USER4 2147483648



#define O_LOCK O_NOCTTY


//These flage are used to tell FDSelect whether to watch a stream for read, write, or both
#define SELECT_READ 1
#define SELECT_WRITE 2


//These flags are used to alter behavior of STREAMSendFile. 
#define SENDFILE_KERNEL 1   //enables use of the 'sendfile' kernel syscall
#define SENDFILE_LOOP   2   //keep copying bytes until required number transfered
#define SENDFILE_NOREAD 4   //
#define SENDFILE_FLUSH  8   //



typedef struct
{
    int Type;
    int in_fd, out_fd;
    unsigned int Flags;
    unsigned int State;
    unsigned int Timeout;
    unsigned int BlockSize;
    unsigned int BuffSize;
    unsigned int StartPoint;

    unsigned int InStart, InEnd;
    unsigned int OutEnd;
    unsigned char *InputBuff;
    unsigned char *OutputBuff;

    unsigned long Size;
    unsigned long BytesRead;
    unsigned long BytesWritten;
    char *Path;
    ListNode *ProcessingModules;
    ListNode *Values;
    ListNode *Items;
} STREAM;


#ifdef __cplusplus
extern "C" {
#endif

//some functions that work on basic file descriptors

//watch a file descriptor for activity. 'Flags' can be SELECT_READ, and/or SELECT_WRITE depending on what is being watched for
int FDSelect(int fd, int Flags, struct timeval *tv);

//is file ready to recieve bytes?
int FDIsWritable(int fd);

//are bytes available to be read? 
int FDCheckForBytes(int fd);


//From here on in it's all functions that effect STREAM objects

//create a STREAM object
STREAM *STREAMCreate();

//the main function to open a stream, see details above
STREAM *STREAMOpen(const char *Path, const char *Config);

//create a stream from a file descriptor
STREAM *STREAMFromFD(int fd);

//create a stream from two file descriptors, one for input/read and one for output/write 
//for example, file descriptors 0 and 1 are normally standard in and standard out and you
//could use this function to create a stream for interacting with the terminal
STREAM *STREAMFromDualFD(int in_fd, int out_fd);


void STREAMSetFlags(STREAM *S, int Set, int UnSet);

//set how long STREAMReadBytes or STREAMReadLine will wait for bytes to arrive. A value of 0 means 'forever'
void STREAMSetTimeout(STREAM *, int Centisecs);

/* configures flushing on a stream. The 'Type' argument can be one of:

FLUSH_FULL         Only flush a streams output buffer when it's full 
FLUSH_LINE         flush on a newline
FLUSH_BLOCK        flush when hit BlockSize
FLUSH_ALWAYS       flush on every call to STREAMWrite or STREAMWriteLine etc
FLUSH_BUFFER       

'StartPoint' sets a number of bytes where flushing starts. This can be used for buffering in media streams, no flushing occurs until
the first time that the output buffer fills to this point, after that flushing happens as normal. Set to zero to disable this feature

'BlockSize' specifies that output should happen in blocks of a certain size. Set to zero to disable this feature.
*/
void STREAMSetFlushType(STREAM *Stream, int Type, int StartPoint, int BlockSize);


//normally you wouldn't use this function it's called internally from STREAMOpen. If you want to open files, and only files, regardless of
//any prefix, then you might use this function. The prefix would be treated as part of the file path, so you could create files called
//'http:myhost'
STREAM *STREAMFileOpen(const char *Path, int Flags);

//Free a stream object without closing any associated file descriptors (e.g. if you're using a stream to read from stdin, but don't want
//to close stdin, use this function  to free the STREAM object
void STREAMDestroy(void *S);

//close a stream and free most associated data, but don't destroy/free the stream object.
//you would not normally use this except if you were linking libUseful to some kind of 
//environment that expects to garbage-collect destroyed items itself (libUseful-lua is an
//example of this situation)
void STREAMShutdown(STREAM *Stream);

//Close a file/connection and free the STREAM object
void STREAMClose(STREAM *Stream);

//write any data stored in the buffers of the stream object to it's associated file/connection
int STREAMFlush(STREAM *Stream);

//clear any data in the input and output buffers of a stream
void STREAMClear(STREAM *Stream);

//return read/write postition of a file 
uint64_t STREAMTell(STREAM *Stream);

//set read/write position of a file. 'whence' takes the same values as lseek
uint64_t STREAMSeek(STREAM *Stream, int64_t offset, int whence);

//read a single character from a stream. Returns STREAM_CLOSED (-1) on stream end. Any other negative
//value is a stream error like STREAM_TIMEOUT and friends. Actual chars are always positive values
int STREAMReadChar(STREAM *);

int STREAMReadUint16(STREAM *S, long *RetVal);
int STREAMReadUint32(STREAM *S, long *RetVal);

//same as STREAMReadChar except this doesn't remove a character from the stream buffer, leaving it
//there to be read again
int STREAMPeekChar(STREAM *);

//write a character to a stream
int STREAMWriteChar(STREAM *,unsigned char c);


//Read 'ByteCount' bytes from a stream into 'Buffer'. Buffer must be large enough to take the specified number of bytes.
//Number of bytes actually read, which can be less than number requested, is returned in 'BytesRead'. 
//Return value of function is a state value that is one of

// STREAM_BYTES_READ    - data was read successfully
// STREAM_TIMEOUT       - we timed out waiting for data (quiet socket)
// STREAM_CLOSED        - end-of-file or connection closed
// STREAM_NODATA        - we didn't time out, but we know there's no data to be read
// STREAM_MESSAGE_END   - we reached the end of a 'message' block
// STREAM_DATA_ERROR    - something else went wrong

//This function is intended for message oriented streams,  (e.g. websocket).
//Keep reading bytes until 'STREAM_MESSAGE_END' is returned, and you will have the full message. 
//Even when 'STREAM_MESSAGE_END' is returned, some bytes may have been read, 
//so check 'BytesRead' to see how many bytes have been read into the buffer. 
//For instance, short messages may return STREAM_MESSAGE_END straight away, 
//because the whole message has been read in one call.
int STREAMReadMessage(STREAM *S, char *Buffer, int Buffsize, int *BytesRead);


//Read 'ByteCount' bytes from a stream into 'Buffer'. Buffer must be large enough to take the specified number of bytes.
//Return value is number of bytes actually read, which can be less than number requested. Negative return values 
//indicate errors or end of stream.
int STREAMReadBytes(STREAM *, char *Buffer, int ByteCount);


//Read 'ByteCount' bytes from a stream into 'Buffer', but do not remove them from the STREAM objects internal buffer, so they
//are still there to be read again. Buffer must be large enough to take the specified number of bytes
//Return value is number of bytes actually read, which can be less than number requested. Negative return values 
//indicate errors or end of stream.
int STREAMPeekBytes(STREAM *S, char *Buffer, int ByteCount);

//Queue 'ByteCount' bytes to be written. STREAMWriteBytes always queues all the bytes you supply, then writes them out when
//a Flush is called (which can happen manually by calling STREAMFlush, or else happens depending on the setting set in
//STREAMSetFlushType
int STREAMWriteBytes(STREAM *, const char *Buffer, int ByteCount);


//Read 'ByteCount' bytes into Buffer, unless terminator character 'Term' is encountered first. Otherwise STREAMReadBytesToTerm
//functions like STREAMReadBytes
int STREAMReadBytesToTerm(STREAM *S, char *Buffer, int ByteCount, unsigned char Term);



//The following block of functions all take a 'Buffer' passed in, and also return it back out. These functions resize the 
//supplied buffer as needed. If there are no more bytes to be read (stream is closed etc) then they will free the buffer
//and return NULL. Thus you do not need to free the buffer if the function returns NULL, that has already been done.
//Best to use 'DestroyString' to free the buffer variable, as it detects NULL and does nothing, whereas 'free' will try
//to free a NULL and crash

//Read bytes until terminator character 'Term' is encountered. 
char* STREAMReadToTerminator(char *Buffer, STREAM *S, unsigned char Term);

//read bytes until any character in the string 'Terms' is encountered
char* STREAMReadToMultiTerminator(char *Buffer, STREAM *S, char *Terms);

//read bytes until newline is encountered
char* STREAMReadLine(char *Buffer, STREAM *S);

//read bytes until end-of-file/stream closed/message end. 
//Use this for reading all of an http document, all of a file
//or reading text-based 'messages' in protocols like websocket.
char *STREAMReadDocument(char *RetStr, STREAM *S);

//read till the string 'Term' is found. Return value is true or false depending on whether the string was found. 
//'RetStr' is resized to accept the bytes read and 'len' is set to the length of those bytes. As this function returns a
//length it can return binary data
int STREAMReadToString(STREAM *S, char **RetStr, int *len, const char *Term);


//write a string to a STREAM without flushing (unless stream buffers are full, when a flush is forced)
int STREAMWriteString(const char *Buffer, STREAM *S);

//write a string to a STREAM and flush if Flush Type is FLUSH_LINE
int STREAMWriteLine(const char *Buffer, STREAM *S);

//Check if there are bytes waiting to be read, returns zero (False) if not non-zero (true) if there are
int STREAMCheckForBytes(STREAM *);

//check if a character is present amoung the bytes waiting to be read
int STREAMCheckForWaitingChar(STREAM *S,unsigned char check_char);

//return the number of bytes waiting in a stream's input buffer
int STREAMCountWaitingBytes(STREAM *);


//given a list of streams, and a timeout value (if NULL then never timeout) this function will perform a select poll
//on those streams and return one that is ready for reading from
STREAM *STREAMSelect(ListNode *Streams, struct timeval *timeout);



//Push bytes into the front of the stream (so, they will be the first things read). 
void STREAMInsertBytes(STREAM *S, const char *Bytes, int len);

//if the stream is a file then peform file locking. Flags are the same as for flock: 
//LOCK_EX for an exclusive lock where only one process can lock the file
//LOCK_SH for a shared lock where many processes can lock, but none can lock exclusively, usually used to implement 'read locks'
//LOCK_UN for unlock
//LOCK_NB for 'non blocking' lock that will fail if file already locked.
//returns TRUE if lock succeeds, FALSE on failure
int STREAMLock(STREAM *S, int flags);

//find the key 'Item' in a file made up of lines broken up by newline, and a key value on each line separated from the rest of the line by 'Delimiter'.
//The entire line is returned, which can then be broken up into key and value
//if SF_SORTED is set in the streams flags (either by STREAMOpen("/tmp/sortedfile.txt","rS")  or (STREAMSetFlags(S, SF_SORTED, 0) ) then a binary search
//will be used to find data in the file, which is MUCH faster. Returns 'true' if key is found, false otherwise.
int STREAMFind(STREAM *S, const char *Item, const char *Delimiter, char **RetStr);

//set and get string properties on a STREAM object.
ListNode *STREAMSetValue(STREAM *S, const char *Name, const char *Value);
char *STREAMGetValue(STREAM *S, const char *Name);

//book any type of object against a STREAM object. 
void STREAMSetItem(STREAM *S, const char *Name, void *Item);
void *STREAMGetItem(STREAM *S, const char *Name);

//copy bytes from 'In' to 'Out' uptil Max bytes. If Max is zero then no maximum will be applied.
//Flags can be a combination of
//SENDFILE_KERNEL   use kernel syscalls for higher efficiency if possible
//SENDFILE_LOOP     keep transferring bytes until 'Max' bytes are transferred, rather than just transferring the number available

unsigned long STREAMSendFile(STREAM *In, STREAM *Out, unsigned long Max, int Flags);

//Copy all bytes from 'Src' to a new file at 'DestPath'
unsigned long STREAMCopy(STREAM *Src, const char *DestPath);

// resize a streams internal buffers to 'size' bytes long. 'Flags' can be SF_SECURE for a secure stream, or just zero otherwise
void STREAMReAllocBuffer(STREAM *S, int size, int Flags);

//convinience macro without 'Flags' argument
#define STREAMResizeBuffer(S, size) (STREAMReAllocBuffer((S), (size), (S->Flags)))


//use this macro to check if a non-blocking stream is ready for write.
//For reading use STREAMCheckForBytes
#define STREAMIsWriteable(S) (FDIsWritable((S)->out_fd))


//This function exists for a very obscure situation where you're using DataProcessors and need to add one, and restream data in the
//stream buffers through it. Most users will never use this.
void STREAMResetInputBuffers(STREAM *S);

void STREAMTruncate(STREAM *S, long size);

//This is used in communication types that require a 'commit' after a transaction. This differs from a 'flush' which just writes
//data to the stream. Currently the only use of this is with HTTP POST, to declare that all uploaded data has been written and that
//the server should process it and send a reply
int STREAMCommit(STREAM *S);


//this is used internally. It is the function that finally pushes bytes onto the wire for a basic file-descriptor connection
//you would never use this except if you were implementing a protocol stack within libUseful
int STREAMWaitForBytes(STREAM *S);
int STREAMPushBytes(STREAM *S, const char *Data, int DataLen);
int STREAMPullBytes(STREAM *S, char *Data, int DataLen);

#ifdef __cplusplus
}
#endif



#endif
