/*
Copyright (c) 2025 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: GPL-3.0
*/

/* This implements a terminal-based calendar widget
 *
 * Example Usage:

STREAM *Term;
char *Tempstr=NULL;
int i;

Term=STREAMFromDualFD(0,1);
TerminalInit(Term, TERM_RAWKEYS | TERM_SAVEATTRIBS);
TerminalClear(Term);

Tempstr=TerminalCalendar(Tempstr, 0, 0, Term, "");
printf("Selected date: %s\n", Tempstr);

TerminalReset(Term);
}


The date returned by 'TerminalCalendar' is in the format 'YYYY-mm-dd'

 *
 *  The 'Config' parameter of TerminalCalendar or TerminalCalendarCreate takes the following entries:
 *
 *  month=<num>                 Month in year, expressed as a number 1-12 (defaults to current month)
 *  year=<num>                  Year (defaults to current year)
 *  x=<col>                     Position the calendar at terminal column 'x'. 
 *  y=<row>                     Position the calendar at terminal row 'y'. 
 *  left_cursor=<str>           Text/Attributes for the left hand side of the cursor
 *  right_cursor=<str>          Text/Attributes for the right hand side of the cursor
 *  TitleAttributes=<str>       Attributes for the Month/Year title bar
 *  TitleMonthAttribs=<str>     Attributes for the Month section of the title bar
 *  TitleYearAttribs=<str>      Attributes for the Year section of the title bar
 *  InsideMonthAttribs=<str>    Attributes for the days within the monthe
 *  OutsideMonthAttribs=<str>   Attributes for the days in previous and next month
 *  TodayhAttribs=<str>         Attributes for date that matches 'Today'
 *  DayHeaders=<csv>            Comma-seperated list of day 'headers', defaults to: Sun,Mon,Tue,Wed,Thu,Fri,Sat
 *  MonthNames=<csv>            Comma-seperated list of month names, defaults to: January,Febuary,March,April,May,June,July,August,September,October,November
 *
 *
 * In the above settings 'Attributes' refers to tilde-formatted values that set colors and terminal attributes
 */


#ifndef LIBUSEFUL_TERMINAL_CALENDAR_H
#define LIBUSEFUL_TERMINAL_CALENDAR_H

#include "includes.h"
#include "Unicode.h"
#include "KeyCodes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "Terminal.h"
#include "TerminalWidget.h"

#define TERMCALENDAR TERMWIDGET

#define TerminalCalendarDestroy TerminalWidgetDestroy
#define TerminalCalendarSetOptions TerminalWidgetSetOptions

TERMCALENDAR *TerminalCalendarCreate(STREAM *Term, int x, int y, const char *Config);
void TerminalCalendarDraw(TERMCALENDAR *TC);
char *TerminalCalendarProcess(char *RetStr, TERMCALENDAR *TC);
char *TerminalCalendar(char *RetStr, STREAM *Term, int x, int y, const char *Config);


#ifdef __cplusplus
}
#endif

#endif
