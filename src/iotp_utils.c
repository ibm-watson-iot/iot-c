/*******************************************************************************
 * Copyright (c) 2018-2019 IBM Corp.
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

#include <MQTTReasonCodes.h>

#include "iotp_version.h"
#include "iotp_utils.h"
#include "iotp_rc.h"

/* 
 * Structure with error/return code description.
 * If you add/update an RC in iotp_rc.h file, make required changes in this structure.
 */
static struct {
    IOTPRC      rc;
    const char * desc;
} rcDesc[] = {
    { IOTPRC_SUCCESS,                  "WIoTP client oprration has completed successfully." },
    { IOTPRC_FAILURE,                  "WIoTP client oprration has failed to complete." },
    { IOTPRC_NOMEM,                    "System memory is not available." },
    { IOTPRC_FILE_OPEN,                "Could not open the specified file." },
    { IOTPRC_LOGGING_INITED,           "Logging is already initialized." },
    { IOTPRC_INVALID_HANDLE,           "WIoTP client handle is NULL or not initialized." },
    { IOTPRC_MISSING_INPUT_PARAM,      "A required configuration parameter is not specified." },
    { IOTPRC_INVALID_PARAM,            "An invalid configuration parameter is specified." },
    { IOTPRC_PARAM_NULL_VALUE,         "NULL or empty value for the configuration parameter is specified." },
    { IOTPRC_PARAM_INVALID_VALUE,      "The value specified for the configuration parameter is not valid." },
    { IOTPRC_QUICKSTART_NOT_SUPPORTED, "WIoTP quickstart sandbox is not supported for the requested operation." },
    { IOTPRC_INVALID_ARGS,             "An invalid argument is specified for the API." },
    { IOTPRC_ARGS_NULL_VALUE,          "The argument value for the API is NULL or empty value." },
    { IOTPRC_ARGS_INVALID_VALUE,       "The specified argument value for the API is invalid." },
    { IOTPRC_CERT_CALLBACK,            "The client certificate callback failed with an error." },
    { IOTPRC_HANDLE_IN_USE,            "The handle is in use." },
    { IOTPRC_NOT_FOUND,                "The file does not exist or is not accessible." },
    { IOTPRC_NOT_CONNECTED,            "WIoTP client is not connected to the platform." },
    { IOTPRC_TIMEOUT,                  "WIoTP client action failed to complete in configured time." },
    { IOTPRC_HANDLER_NOT_FOUND,        "No message callback handler is configured. Can not process the message." },
    { IOTPRC_DM_ACTION_STARTED,        "WIoTP device management action is started." },
    { IOTPRC_DM_ACTION_FAILED,         "WIoTP device management action has failed to start." },
    { IOTPRC_DM_ACTION_NOT_SUPPORTED,  "WIoTP device management action is not supported." }
};
#define NUM_RC (sizeof(rcDesc) / sizeof(rcDesc[0]))

/*
 * Variables/defines used in logging/tracing related functions.
 */
#define MAX_LOG_BUFSIZE 8192
static int versionWritten = 0;
static int logLevel = LOGLEVEL_DEBUG;   /* Default logging level */
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
    }

    return "UNKNOWN";
}


/* Create log entry */
void iotp_utils_log(IoTPLogLevel level, const char * file, const char * func, int line, const char * fmts, ...) 
{
    va_list args;

    if ((int)level <= logLevel) 
    {
        char buf[MAX_LOG_BUFSIZE];
        memset(buf, '0', MAX_LOG_BUFSIZE);

        va_start(args, fmts);
        vsnprintf(buf, MAX_LOG_BUFSIZE, fmts, args);
        va_end(args);

        if ( lgh != NULL ) {
            memset(message, '0', maxBufSize);
            snprintf(message, maxBufSize, LogMsgFormat, __TIMESTAMP__, basename((char *)file), func, line, iotp_utils_logLevelStr(level), buf);
            (*lgh)(level, message);
        } else  if (logger) {
            fprintf(logger, "%s %s %s %d: %s: %s\n", __TIMESTAMP__, basename((char *)file), func, line, iotp_utils_logLevelStr(level), buf);
            fflush(logger);
        } else {
            fprintf(stdout, "%s %s %s %d: %s: %s\n", __TIMESTAMP__, basename((char *)file), func, line, iotp_utils_logLevelStr(level), buf);
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

/* Trim leading white characters */
char * iotp_utils_trim(char *str) 
{
    int i;
    int j = 0;

    if ( !str || *str == '\0' ) return NULL;

    while (str[j] == ' ' || str[j] == '\r' || str[j] == '\n') j++;

    if (j != 0) {
        i = 0;
        while (str[i+j] != '\0') {
            str[i] = str[i+j];
            i++;
        }
        str[i] = '\0';
    }
    return str;
}

/* Tokenize string based on leading and trailing characters */
char * iotp_utils_getToken(char * from, const char * leading, const char * trailing, char * * more) 
{
    char * ret; if (!from)
        return NULL;
    while (*from && strchr(leading, *from))
        from++;
    if (!*from) {
        if (more)
            *more = NULL;
        return NULL;
    }
    ret = from;
    while (*from && !strchr(trailing, *from))
        from++;
    if (*from) {
        *from = 0;
        if (more)
            *more = from + 1;
    } else {
        if (more)
            *more = NULL;
    }
    return ret;
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

    LOG(DEBUG, "uuid_str = %s",uuid_str);
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

/* check if file exist */
IOTPRC iotp_utils_fileExist(const char * filePath)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    if (filePath == NULL) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        return rc;
    }

    if ( access( filePath, R_OK ) != -1 ) {
        rc = IOTPRC_NOT_FOUND;
        return rc;
    }

    return rc;
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
            /* printf("dli_sname: %s\n", info.dli_sname); */
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
IOTPRC iotp_utils_setLogHandler(IoTPLogTypes type, void * handler) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    if ( handler == NULL ) {
        LOG(WARN, "NULL Log handler is specified.");
        logger = stdout;
        rc = IOTPRC_PARAM_NULL_VALUE;
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
                rc = IOTPRC_PARAM_INVALID_VALUE;
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
                rc = IOTPRC_PARAM_INVALID_VALUE;
                LOG(WARN, "Invalid Log FilePointer is specified.");
            }
        } else if ( type == IoTPLog_FileDescriptor ) {
            if ( hType == 2 || hType == 3 ) {
                int *fdaddr = (int *)handler;
                int fd = *fdaddr;
                lgh = NULL;
                logger = fdopen(fd, "a");
                iotp_utils_writeClientVersion();
                LOG(INFO, "Log handler is set to a file descriptor.");
            } else {
                lgh = NULL;
                logger = stdout;
                rc = IOTPRC_PARAM_INVALID_VALUE;
                LOG(WARN, "Invalid Log FileDescriptor handle is specified.");
            }
        } else {
            logger = stdout;
            rc = IOTPRC_PARAM_INVALID_VALUE;
            LOG(WARN, "Invalid Log handler type is specified.");
        }
    }

    return rc;
}

 
/* IOTPRC_toString() API returns WIoTP client reason code description. */
const char * IOTPRC_toString(IOTPRC rc)
{
    int i = 0;
    const char *desc = NULL;

    /* check for MQTTAsync errors - will be less than 0 */
    if ( rc < IOTPRC_SUCCESS ) {
        desc = "Error returned by Paho Asynchronous MQTT Client library.";
        return desc;
    }

    /* check for IOTPRC_SUCCESS and IOTPRC_FAILURE */
    if ( rc == IOTPRC_SUCCESS || rc == IOTPRC_FAILURE ) {
        desc = rcDesc[rc].desc;
        return desc;
    }

    /* If rc is less than 1001, then it could be Paho MQTT Client library reason code */
    if ( rc < 1001 ) {
        desc = MQTTReasonCode_toString((enum MQTTReasonCodes)rc);
    } else {
        /* IoTP Client error or reason code */
        for (i=0; i<NUM_RC; i++) {
            if (rcDesc[i].rc == rc) {
                desc = rcDesc[i].desc;
                break;
            }
        }
    }

    /* check for unknown error or reason code */
    if ( desc == NULL ) {
        desc = "Unknown error or reason code";
    }

    return desc;
}


