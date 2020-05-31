#ifndef LIBUSEFUL_TERMINAL_MENUBAR_H
#define LIBUSEFUL_TERMINAL_MENUBAR_H

#include "includes.h"
#include "Unicode.h"
#include "KeyCodes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "Terminal.h"
#include "TerminalMenu.h"

#define TerminalMenuBarCreate TerminalWidgetCreate
#define TerminalMenuBarDestroy TerminalWidgetDestroy
#define TerminalMenuBarSetOptions TerminalWidgetSetOptions

char *TerminalMenuBarOnKey(char *RetStr, TERMMENU *MB, int key);
char *TerminalMenuBar(char *RetStr, STREAM *Term, const char *Config);


#ifdef __cplusplus
}
#endif

#endif
