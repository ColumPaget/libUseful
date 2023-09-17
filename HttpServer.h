#ifndef LIBUSEFUL_HTTP_SERVER_H
#define LIBUSEFUL_HTTP_SERVER_H

#include "Stream.h"
#include "Vars.h"


#ifdef __cplusplus
extern "C" {
#endif


void HTTPServerParseClientCookies(ListNode *Vars, const char *Str);
void HTTPServerParseAuthorization(ListNode *Vars, const char *Str);
void HTTPServerParseClientHeaders(STREAM *S);
void HttpServerSendHeaders(STREAM *S, int ResponseCode, const char *ResponseText, const char *Headers);

#ifdef __cplusplus
}
#endif


#endif
