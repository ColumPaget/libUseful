#ifndef LIBUSEFUL_SMTP_H
#define LIBUSEFUL_SMTP_H

#include "includes.h"
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

int SendMail(const char *Sender, const char *Recipient, const char *Subject, const char *Body);

#ifdef __cplusplus
}
#endif


#endif
