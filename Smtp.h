/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef LIBUSEFUL_SMTP_H
#define LIBUSEFUL_SMTP_H

#include "includes.h"
#include "defines.h"

//Flags that pass as the 'Flags' argument of SMTPSendMail
#define LU_SMTP_NOSSL       1   //don't use SSL/TLS (including starttls even if server supports it)
#define LU_SMTP_NOTLS       1   //don't use SSL/TLS (including starttls even if server supports it)
#define LU_SMTP_NOHEADER    2   //don't add a header to the mail
#define LU_SMTP_STARTTLS    4   //use 'starttls' if server supports it
#define LU_SMTP_INITIAL_TLS 8   //use TLS/SSL straight away (rather than 'starttls')
#define LU_SMTP_INITIAL_SSL 8   //use TLS/SSL straight away (rather than 'starttls')

#ifdef __cplusplus
extern "C" {
#endif

STREAM *SMTPConnectServer(const char *ServerURL, const char *Sender, const char *Recipients, int Flags);
STREAM *SMTPConnect(const char *Sender, const char *Recipients, int Flags);
int SMTPSendMailViaServer(const char *ServerURL, const char *Sender, const char *Recipient, const char *Subject, const char *Body, int Flags);
int SMTPSendMail(const char *Sender, const char *Recipient, const char *Subject, const char *Body, int Flags);
int SMTPSendMailFileViaServer(const char *ServerURL, const char *Sender, const char *Recipient, const char *Path, int Flags);
int SMTPSendMailFile(const char *Sender, const char *Recipient, const char *Path, int Flags);
char *SMTPRead(char *RetStr, STREAM *S);

#ifdef __cplusplus
}
#endif


#endif
