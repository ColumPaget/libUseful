#ifndef LIBUSEFUL_LINEEDIT_H
#define LIBUSEFUL_LINEEDIT_H

/* 
This module supplies a line-editor with a history. 

Example code using this can be found in Terminal.h in 'TerminalReadText' or in examples/EditLine.c

The 'LineEditCreate' command can be passed the flags:

LINE_EDIT_HISTORY    - provide an recallable history of entered lines
LINE_EDIT_NOMOVE     - don't allow use of left/right to move back into the text
                       this is used for password entry text that is invisible and
                       thus we cannot allow the user to use full editing, as they
                       can become 'lost'.

The 'EditLineHandleChar' function does most of the work, it expects to receive
the following characters (defined in KeyCodes.h)


TKEY_LEFT      -    move cursor left within text
TKEY_RIGHT     -    move cursor right within text
TKEY_UP        -    if history mode is active, recall previous entry
TKEY_DOWN      -    if moving through history goto next entry
TKEY_HOME      -    move cursor to start of text
TKEY_END       -    move cursor to end of text
TKEY_BACKSPACE -    backspace delete previous character
TKEY_ERASE     -    erase curr character
TKEY_ENTER     -    return/enter line
TKEY_ESCAPE    -    cancel line editing

The 'EditLineHandleChar' function returns the cursor position within the edited line, or if 'enter' is pressed it will return the value LINE_EDIT_ENTER, if 'escape' is pressed then it will return LINE_EDIT_CANCEL.

When LINE_EDIT_ENTER is called 'LineEditDone' should be used to collect the typed line, as it will reset the LineEdit for a new line to be typed in, and if history mode is active, it will add the typed line to the history.
*/


#include "includes.h"
#include "List.h"


#define LINE_EDIT_ENTER  -1
#define LINE_EDIT_CANCEL -2

#define LINE_EDIT_HISTORY    1
#define LINE_EDIT_NOMOVE     2

typedef struct
{
char *Line;
int Flags;
int Len;
int MaxHistory;
int Cursor;
ListNode *History;
} TLineEdit;


#define LineEditSetMaxHistory(LE, Max) ((LE)->MaxHistory = (Max))

TLineEdit *LineEditCreate(int Flags);
void LineEditSetText(TLineEdit *LE, const char *Text);
void LineEditDestroy(TLineEdit *LE);
int LineEditHandleChar(TLineEdit *LE, int Char);
char *LineEditDone(char *RetStr, TLineEdit *LE);
void LineEditClearHistory(TLineEdit *LE);
void LineEditAddToHistory(TLineEdit *LE, const char *Text);

#endif
