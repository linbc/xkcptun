/* vim: set et ts=4 sts=4 sw=4 : */
/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/** @file debug.c
    @brief Debug output routines
    @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
*/

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>

#include "debug.h"

debugconf_t debugconf = {
    .debuglevel = LOG_INFO,
    .log_stderr = 1,
    .log_syslog = 0,
    .syslog_facility = LOG_USER
};


#ifdef WIN32

#include <strsafe.h>	//for StringCchPrintfA
#include <Windows.h>	//for SYSTEMTIME
#include <process.h>	//for getpid

void Time_tToSystemTime(time_t dosTime, SYSTEMTIME *systemTime)
{
	LARGE_INTEGER jan1970FT;
	LARGE_INTEGER utcFT;
	jan1970FT.QuadPart = 116444736000000000LL; // january 1st 1970
	utcFT.QuadPart = ((unsigned __int64)dosTime) * 10000000 + jan1970FT.QuadPart;

	FileTimeToSystemTime((FILETIME*)&utcFT, systemTime);
}

char* ctime_r(const time_t *t, char *buf)
{
	SYSTEMTIME systime;
	const char * const dayOfWeek[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	const char * const monthOfYear[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	Time_tToSystemTime(*t, &systime);
	/* We don't know how long `buf` is, but assume it's rounded up from the minimum of 25 to 32 */
	StringCchPrintfA(buf, 32, "%s %s %d %02d:%02d:%02d %04d", dayOfWeek[systime.wDayOfWeek - 1], monthOfYear[systime.wMonth - 1],
		systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wYear);
	return buf;
}

#endif // WIN32


/** @internal
Do not use directly, use the debug macro */
void
_debug(const char *filename, int line, int level, const char *format, ...)
{
    char buf[28];
    va_list vlist;
    time_t ts;
    //sigset_t block_chld;

    time(&ts);

    if (debugconf.debuglevel >= level) {
        //sigemptyset(&block_chld);
        //sigaddset(&block_chld, SIGCHLD);
        //sigprocmask(SIG_BLOCK, &block_chld, NULL);

        if (level <= LOG_WARNING) {
            fprintf(stderr, "[%d][%.24s][%u](%s:%d) ", level, ctime_r(&ts, buf), getpid(),
                filename, line);
            va_start(vlist, format);
            vfprintf(stderr, format, vlist);
            va_end(vlist);
            fputc('\n', stderr);
        } else if (debugconf.log_stderr) {
            fprintf(stderr, "[%d][%.24s][%u](%s:%d) ", level, ctime_r(&ts, buf), getpid(),
                filename, line);
            va_start(vlist, format);
            vfprintf(stderr, format, vlist);
            va_end(vlist);
            fputc('\n', stderr);
        }
#ifndef WIN32
        if (debugconf.log_syslog) {
            openlog("xkcptun", LOG_PID, debugconf.syslog_facility);
            va_start(vlist, format);
            vsyslog(level, format, vlist);
            va_end(vlist);
            closelog();
        }
#endif // !WIN32

        //sigprocmask(SIG_UNBLOCK, &block_chld, NULL);
    }
}
