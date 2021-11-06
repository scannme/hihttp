//
// Create by lituanjie on 2021-11-6
// The log file 
//
//
#include <stdarg.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

#define  MAXLINE 4096

/*error print a diagnostic and optionally exit*/
void error(int status, int err, char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);

    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (err) {
        fprintf(stderr, ": %s (%d)\n", strerror(err), err);
    }

    if (status) {
        exit(status);
    }
}

static void
err_doit(int errnoflag, int level, const char *fmt, va_list ap) {

}

void sys_log(int serverity, const char *msg) {
    const char *serverity_srt;
    switch (serverity) {
        case LOG_DEBUG_TYPE:
            serverity_srt = "debug";
            break;
        case LOG_MSG_TYPE:
            serverity_srt = "msg";
            break;
        case LOG_WARN_TYPE:
            serverity_srt = "warn";
            break;
        case LOG_ERR_TYPE:
            serverity_srt = "err";
            break;
        default:
            serverity_srt = "???";
            break;

    }
    (void) fprintf(stdout, "[%s] %s\n", serverity_srt, msg);
}

void sys_logx(int severity, const char *errstr, const char *fmt, va_list ap)
{
    char buf[1024];
    size_t len;

    if (fmt != NULL)
        vsnprintf(buf, sizeof(buf), fmt, ap);
    else
        buf[0] = '\0';

    if (errstr) {
        len = strlen(buf);
        if (len < sizeof(buf) - 3) {
            snprintf(buf + len, sizeof(buf) - len, ": %s", errstr);
        }
    }

    sys_log(severity, buf);
}

void sys_msgx(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    sys_logx(LOG_MSG_TYPE, NULL, fmt, ap);
    va_end(ap);
}

