#ifndef LIBUSEFUL_WEBSOCKET_H
#define LIBUSEFUL_WEBSOCKET_H

#include "Stream.h"

int WebSocketSendBytes(STREAM *S, const char *Data, int Len);
int WebSocketReadBytes(STREAM *S, char *Data, int Len);
STREAM *WebSocketOpen(const char *URL, const char *Config);

#endif
