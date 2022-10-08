#ifndef LIBUSEFUL_TERMINAL_KEYS_H
#define LIBUSEFUL_TERMINAL_KEYS_H

#include "includes.h"

const char *TerminalTranslateKeyCode(int key);
int TerminalTranslateKeyStrWithMod(const char *str, int *mod);
int TerminalTranslateKeyStr(const char *str);
int TerminalReadCSISeq(STREAM *S);
int TerminalReadSSCSeq(STREAM *S);

#endif

