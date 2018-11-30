/*******************************************************************************
 * Copyright (c) 2017-2018 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 *
 * Contrinutors:
 *    Ranjan Dasgupta         - Initial drop
 *
 *******************************************************************************/

#include "iotp_version.h"
#include "iotp_utils.h"

#define MAX_LOG_BUFSIZE 8192

static int versionWritten = 0;
static int logLevel = LOGLEVEL_TRACE;   /* Default logging level */
FILE *logger = NULL;
IoTPLogHandler *lgh = NULL;
char *LogMsgFormat = "%s %s %s %d: %s: %s\n";
int maxBufSize = 8500;
char message[8500];

/* LOG Level string */
static char * iotp_utils_logLevelStr(int level)
{
    switch(level) {
        case LOGLEVEL_ERROR: return "ERROR";
        case LOGLEVEL_WARN: return "WARN";
        case LOGLEVEL_INFO: return "INFO";
        case LOGLEVEL_DEBUG: return "DEBUG";
        case LOGLEVEL_TRACE: return "TRACE";
    }

    return "UNKNOWN";
}

/* Create log entry */
void iotp_utils_logInvoke(IoTPLogLevel level, const char * func, const char * file, int line, const char * fmts, ...) 
{
    va_list args;
    char buf[MAX_LOG_BUFSIZE];

    memset(buf, '0', MAX_LOG_BUFSIZE);
    memset(message, '0', maxBufSize);

    if ((int)level <= logLevel) 
    {
        va_start(args, fmts);
        vsnprintf(buf, MAX_LOG_BUFSIZE, fmts, args);
        va_end(args);

        snprintf(message, maxBufSize, LogMsgFormat, __TIMESTAMP__, func, basename((char *)file), line, iotp_utils_logLevelStr(level), buf);

        if ( lgh != NULL ) {
            (*lgh)(level, message);
        } else  if (logger) {
            fprintf(logger, "%s %s %s %d: %s: %s\n", __TIMESTAMP__, func, basename((char *)file), line, iotp_utils_logLevelStr(level), buf);
            fflush(logger);
        } else {
            fprintf(stdout, "%s %s %s %d: %s: %s\n", __TIMESTAMP__, func, basename((char *)file), line, iotp_utils_logLevelStr(level), buf);
            fflush(stdout);
        }
    }
}

/* Set log level */
void iotp_utils_setLogLevel(IoTPLogLevel level)
{
    LOG(INFO, "Log Level is set to %d", level);
    logLevel = level;
}

/* Trim characters */
char * iotp_utils_trim(char *str) 
{
    size_t len = 0;
    char *frontp = str - 1;
    char *endp = NULL;

    if (str == NULL)
        return NULL;

    if (str[0] == '\0')
        return str;

    len = strlen(str);
    endp = str + len;

    while (isspace(*(++frontp)))
        ;

    while (isspace(*(--endp)) && endp != frontp)
        ;

    if (str + len - 1 != endp)
        *(endp + 1) = '\0';
    else if (frontp != str && endp == frontp)
        *str = '\0';

    endp = str;
    if (frontp != str) {
        while (*frontp)
            *endp++ = *frontp++;

        *endp = '\0';
    }

    return str;
}

/* Function to free the allocated memory for character string. */
void iotp_utils_freePtr(void * p)
{
    if (p != NULL) {
        free(p);
        p = NULL;
    } 
}

/* generate UUID */
void iotp_utils_generateUUID(char * uuid_str)
{
    LOG(DEBUG, "entry::");

    char GUID[40];
    int t = 0;
    char *szTemp = "xxxxxxxx-xxxxy-4xxxx-yxxxy-xxxxxxxxxxxx";
    char *szHex = "0123456789ABCDEF-";
    int nLen = strlen (szTemp);

    for (t=0; t<nLen+1; t++)
    {
        int r =( rand ())% 16;
        char c = ' ';

        switch (szTemp[t])
        {
            case 'x' : { c = szHex [r]; } break;
            case 'y' : { c = szHex [((r & 0x03) | 0x08)]; } break;
            case '-' : { c = '-'; } break;
            case '4' : { c = '4'; } break;
        }

        GUID[t] = ( t < nLen ) ? c : 0x00;
    }
    strcpy(uuid_str , GUID);

    LOG(DEBUG, "entry:: uuid_str = %s",uuid_str);
}

/* add delay */
void iotp_utils_delay(long milsecs)
{
#if defined(WIN32) || defined(WIN64)
        Sleep(milsecs);
#else
        usleep(milsecs * 1000);
#endif
}

/* Print IoTP Client version information - if not written yet */
void iotp_utils_writeClientVersion(void) 
{
    if ( versionWritten == 0 ) {
        versionWritten = 1;
        LOG(INFO, "Watson IoT Platform C Client - Version: %s", IOTP_CLIENT_VERSION);
        LOG(INFO, "Build Date: %s", BUILD_TIMESTAMP);
    }
}

/* check if pointer is a valid function pointer - return 1 if valid */
int iotp_get_handlerType(void *handler)  
{
    int retval = 3; /* File descriptor */

#if defined(OSX)
    Dl_info info;
    int rc = dladdr(handler, &info);
    if ( rc == 0 ) {
        retval = 3;
    } else {
        if ( info.dli_sname ) {
            printf("dli_sname: %s\n", info.dli_sname);
            if ( strcmp(info.dli_sname, "usual") == 0 || strcmp(info.dli_sname, "__sF") == 0 ) {
                retval = 2; /* File pointer */
            } else {
                retval = 1; /* Function pointer */
            }
        }
    }
#else
    int *faddr = (int *)handler;
    int fd = *faddr;
    if ( fcntl(fd, F_GETFL) == -1 ) {
        if ( fileno(handler) == -1 ) {
            retval = 1; /* Callback pointer */
        } else {
            retval = 2; /* File pointer */
        }
    }
#endif

    return retval;
}


/* Set log handler */
IoTP_RC iotp_utils_setLogHandler(IoTPLogTypes type, void * handler) 
{
    IoTP_RC rc = IoTP_SUCCESS;

    if ( handler == NULL ) {
        LOG(WARN, "NULL Log handler is specified.");
        logger = stdout;
        rc = IoTP_RC_PARAM_NULL_VALUE;
    } else {
        int hType = iotp_get_handlerType(handler);
        if ( type == IoTPLog_Callback ) {
            if ( hType == 1 ) { 
                lgh = (IoTPLogHandler *)handler;
                logger = NULL;
                iotp_utils_writeClientVersion();
                LOG(INFO, "Log handler is set to callback function.");
            } else {
                lgh = NULL;
                logger = stdout;
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(WARN, "Invalid Log FuncPointer is specified.");
            }
        } else if ( type == IoTPLog_FilePointer ) {
            if ( hType == 2 ) { 
                lgh = NULL;
                logger = (FILE *)handler;
                iotp_utils_writeClientVersion();
                LOG(INFO, "Log handler is set to a file pointer.");
            } else {
                lgh = NULL;
                logger = stdout;
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(WARN, "Invalid Log FilePointer is specified.");
            }
        } else if ( type == IoTPLog_FileDescriptor ) {
            if ( hType == 3 ) { 
                int *fdaddr = (int *)handler;
                int fd = *fdaddr;
                lgh = NULL;
                logger = fdopen(fd, "a");
                iotp_utils_writeClientVersion();
                LOG(INFO, "Log handler is set to a file descriptor.");
            } else {
                lgh = NULL;
                logger = stdout;
                rc = IoTP_RC_PARAM_INVALID_VALUE;
                LOG(WARN, "Invalid Log FileDescriptor handle is specified.");
            }
        } else {
            logger = stdout;
            rc = IoTP_RC_PARAM_INVALID_VALUE;
            LOG(WARN, "Invalid Log handler type is specified.");
        }
    }

    return rc;
}

 
