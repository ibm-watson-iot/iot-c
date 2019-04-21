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

/* Forward references */
static int jsonNewEnt(IoTP_json_parse_t * pobj, int objtype, const char * name, const char * value, int level);
static int jsonToken(IoTP_json_parse_t * pobj, char * * data);
static int jsonKeyword(IoTP_json_parse_t * pobj, int otype, const char * match, int len);
static int jsonString(IoTP_json_parse_t * pobj);
static int jsonNumber(IoTP_json_parse_t * pobj);

/* Starter states for UTF8 */
static int States[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, 1,
};

/* Initial byte masks for UTF8 */
static int StateMask[5] = {0, 0, 0x1F, 0x0F, 0x07};

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

/* Check for valid second byte of UTF-8 */
static int validSecond(int state, int byte1, int byte2) {
    int ret = 1;

    if (byte2 < 0x80 || byte2 > 0xbf)
        return 0;

    switch (state) {
    case 2:
        if (byte1 < 2)
            ret = 0;
        break;
    case 3:
        if ((byte1== 0 && byte2 < 0xa0) || (byte1 == 13 && byte2 > 0x9f))
            ret = 0;
        break;
    case 4:
        if (byte1 == 0 && byte2 < 0x90)
            ret = 0;
        else if (byte1 == 4 && byte2 > 0x8f)
            ret = 0;
        else if (byte1 > 4)
            ret = 0;
        break;
    }
    return ret;
}


/* Scan a UTF-8 string for validity. */
static int validUTF8(const char * s, int len) {
    int  byte1 = 0;
    int  state = 0;
    int  count = 0;
    int  inputsize = 0;
    uint8_t * sp = (uint8_t *)s;
    uint8_t * endp = (uint8_t *)(s+len);

    while (sp < endp) {
        if (state == 0) {
            /* Fast loop in single byte mode */
            for (;;) {
                if (*sp >= 0x80)
                    break;
                count++;
                if (++sp >= endp)
                    return count;
            }

            count++;
            state = States[*sp >>3];
            byte1 = *sp & StateMask[state];
            sp++;
            inputsize = 1;
            if (state == 1)
                return -1;
        } else {
            int byte2 = *sp++;
            if ((inputsize==1 && !validSecond(state, byte1, byte2)) ||
                (inputsize > 1 && (byte2 < 0x80 || byte2 > 0xbf)))
                return -1;
            if (inputsize+1 >= state) {
                state = 0;
            } else {
                inputsize++;
            }
        }
    }
    if (state)
        return -1;        /* Incomplete character */
    return count;
}

/* Initialize JSON parse object */
IoTP_json_parse_t * iotp_json_init(int payloadlen, char *payload) {
    IoTP_json_parse_t *pobj = NULL;

    if ( payloadlen < 2 || payload == NULL || *payload == '\0' ) {
        LOG(ERROR, "Invalid JSON string. len=%d payload:%s", payloadlen, payload? payload:"");
        return NULL;
    }
   
    pobj = (IoTP_json_parse_t *) calloc(1, sizeof(IoTP_json_parse_t)); 
    pobj->src_len             = payloadlen;
    pobj->source              = (char *)malloc(payloadlen+1);
    memcpy(pobj->source, payload, payloadlen);
    pobj->source[payloadlen] = 0;

    int rc = iotp_json_parse(pobj);

    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Could not parse JSON object. rc=%d len=%d payload:%s", rc, payloadlen, payload);

        if ( pobj->free_ent )
            free(pobj->ent);

        if ( pobj->source )
            free(pobj->source);

        if (pobj)
            free(pobj);
        pobj = NULL;
    }

    return pobj;
}

/* Free initialized JSON parse object */
int iotp_json_free(IoTP_json_parse_t * pobj) {
    IOTPRC rc = IOTPRC_SUCCESS;

    if ( !pobj ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "Cannot free invalid JSON object.");
    }

    if (pobj->free_ent) 
        free(pobj->ent);

    if ( pobj->source )
        free(pobj->source);

    free(pobj);
    pobj = NULL;

    return rc;
}
        

/* Parse a JSON message. */
int iotp_json_parse(IoTP_json_parse_t * pobj) {
    int    token;
    int    state;
    int    entnum;
    int    where [256];
    int    level = 0;
    int    inarray = 0;

    char * value = NULL;
    char * name;

    /*
     * The initial entry can be an object or an array
     */
    pobj->pos = pobj->source;
    pobj->left = pobj->src_len;
    pobj->line = 1;
    where[0] = 0;
    token = jsonToken(pobj, NULL);
    switch (token) {
    case JTOK_StartObject:  /* Outer object is an object */
        entnum = jsonNewEnt(pobj, JSON_Object, NULL, NULL, level);
        state = JSTATE_Name;
        break;
    case JTOK_StartArray:   /* Outer object is an array */
        entnum = jsonNewEnt(pobj, JSON_Array, NULL, NULL, level);
        state = JSTATE_Value;
        inarray = 1;
        name = NULL;
        break;
    case JTOK_End:           /* Object is empty */
        level = -1;
        state = JSTATE_Done;
        break;
    default:
        /* TODO: Return should indicate invalid entry.
         * And pobj->rc should probably also indicate data is
         * not valid for checks in other places.
         *
         * Forcicing rc = 2 (when rc is not yet set) to indicate
         * JSON message is not valid.
         *
         */
        if (!pobj->rc)
            pobj->rc = 2;
        return pobj->rc;
    }

    /*
     * Loop thru all content
     */
    while (state != JSTATE_Done) {
        switch (state) {

        /*
         * We are expecting the name within an object
         */
        case JSTATE_Name:
            token = jsonToken(pobj, &name);
            switch (token) {
            case JTOK_EndObject:
                 if (inarray) {
                    state = JSTATE_Done;
                    break;
                }
                pobj->ent[where[level]].count = pobj->ent_count-where[level]-1;
                if (--level >= 0) {
                    inarray = pobj->ent[where[level]].objtype == JSON_Array;
					state = JSTATE_Comma;
                } else {
                    state = JSTATE_Done;
                }
                break;

            case JTOK_String:
                token = jsonToken(pobj, NULL);
                if (token != JTOK_Colon) {
                    pobj->rc = 2;
                    state = JSTATE_Done;
                    break;
                }
                state = JSTATE_Value;
                break;
            default:
                state = JSTATE_Done;
                break;
            }
            break;

        /*
         * We are expecting a value
         */
        case JSTATE_Value:
            token = jsonToken(pobj, &value);
            switch (token) {
            case JTOK_String:
                entnum = jsonNewEnt(pobj, JSON_String, name, value, level);
                state = JSTATE_Comma;
                break;
            case JTOK_Integer:
                entnum = jsonNewEnt(pobj, JSON_Integer, name, value, level);
                if (strlen(value)<10)
                    pobj->ent[entnum].count = strtol(value, NULL, 10);
                else
                    pobj->ent[entnum].objtype = JSON_Number;
                state = JSTATE_Comma;
                break;
            case JTOK_Number:
                entnum = jsonNewEnt(pobj, JSON_Number, name, value, level);
                state = JSTATE_Comma;
                break;
            case JTOK_StartObject:
                entnum = jsonNewEnt(pobj, JSON_Object, name, value, level);
                where[++level] = entnum;
                inarray = 0;
                state = JSTATE_Name;
                break;
            case JTOK_StartArray:
                entnum = jsonNewEnt(pobj, JSON_Array, name, NULL, level);
                where[++level] = entnum;
                state = JSTATE_Value;
                name = NULL;
                inarray = 1;
                break;
            case JTOK_True:
                entnum = jsonNewEnt(pobj, JSON_True, name, NULL, level);
                state = JSTATE_Comma;
                break;
            case JTOK_False:
                entnum = jsonNewEnt(pobj, JSON_False, name, NULL, level);
                state = JSTATE_Comma;
                break;
            case JTOK_Null:
                entnum = jsonNewEnt(pobj, JSON_Null, name, NULL, level);
                state = JSTATE_Comma;
                break;
            case JTOK_EndArray:
                if (!inarray) {
                    state = JSTATE_Done;
                    break;
                }
                pobj->ent[where[level]].count = pobj->ent_count-where[level]-1;
                if (--level >= 0) {
                    inarray = pobj->ent[where[level]].objtype == JSON_Array;
					state = JSTATE_Comma;
                } else {
                    state = JSTATE_Done;
                }
                break;
            default:
                state = JSTATE_Done;
                break;
            }
            break;

        /*
         * Comma or end of object
         */
        case JSTATE_Comma:
            token = jsonToken(pobj, NULL);
            switch (token) {
            /* Comma separator between objects */
            case JTOK_Comma:
                state = inarray ? JSTATE_Value : JSTATE_Name;
                break;

            /* Right brace ends an object */
            case JTOK_EndObject:
                if (inarray) {
                    state = JSTATE_Done;
                    break;
                }
                pobj->ent[where[level]].count = pobj->ent_count-where[level]-1;
                if (--level >= 0) {
                    inarray = pobj->ent[where[level]].objtype == JSON_Array;
                    state = JSTATE_Comma;
                } else {
                    state = JSTATE_Done;
                }
                break;

            /* Right bracket ends and array */
            case JTOK_EndArray:
                if (!inarray) {
                    state = JSTATE_Done;
                    break;
                }
                pobj->ent[where[level]].count = pobj->ent_count-where[level]-1;
                if (--level >= 0) {
                    inarray = pobj->ent[where[level]].objtype == JSON_Array;
                    state = JSTATE_Comma;
                } else {
                    state = JSTATE_Done;
                }
                break;

            /* End found.  This is good if level is -1 */
            case JTOK_End:
                state = JSTATE_Done;
                break;

            /* Error */
            default:
                state = JSTATE_Done;
                break;
            }
            break;
        }
    }

    /* Check all objects are complete */
    if (level >= 0) {
        if (!pobj->rc) {
            pobj->rc = 1;
            fprintf(stdout, "Unexpected end of JSON message\n");
        }
    }
    return pobj->rc;
}


/* Make a new entry */
static int jsonNewEnt(IoTP_json_parse_t * pobj, int objtype, const char * name, const char * value, int level) {
    int entnum;
    IoTP_json_entry_t * ent;

    if (pobj->ent_count >= pobj->ent_alloc) {
        int newalloc;
        if (pobj->ent_alloc < 25)
            newalloc = 100;
        else
            newalloc = pobj->ent_alloc*4;
        if (pobj->free_ent) {
            pobj->ent = realloc(pobj->ent, newalloc*sizeof(IoTP_json_entry_t));
        } else {
            IoTP_json_entry_t * tmpbuf = malloc(newalloc * sizeof(IoTP_json_entry_t));
            if (pobj->ent_count)
                memcpy(tmpbuf, pobj->ent, pobj->ent_count * sizeof(IoTP_json_entry_t));
            pobj->ent = tmpbuf;
            pobj->free_ent = 1;
        }
        pobj->ent_alloc = newalloc;
    }
    entnum = pobj->ent_count++;
    ent = pobj->ent+entnum;
    ent->objtype = objtype;
    ent->name    = name;
    ent->value   = value;
    ent->level   = level;
    ent->line    = pobj->line;
    return entnum;
}


/* Returns the next token. */
static int jsonToken(IoTP_json_parse_t * pobj, char * * data) {
    while (pobj->left-- > 0) {
        uint8_t ch = *pobj->pos++;
        switch (ch) {
        case ' ':
        case '\t':
        case '\r':
        case 0x0b:
        case 0x0c:
            break;
        case '\n':
            pobj->line++;
            break;
        case '{':
            return JTOK_StartObject;
        case '}':
            return JTOK_EndObject;
        case '[':
            return JTOK_StartArray;
        case ']':
            return JTOK_EndArray;
        case ':':
            return JTOK_Colon;
        case ',':
            return JTOK_Comma;
        case '"':
            if (data)
                *data = pobj->pos;
            return jsonString(pobj);
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if (data) {
                *data = pobj->pos-2;
                return jsonNumber(pobj);
            }
            return JTOK_Error;
        case 't':
            return jsonKeyword(pobj, JTOK_True, "true", 3);
        case 'f':
            return jsonKeyword(pobj, JTOK_False, "false", 4);
        case 'n':
            return jsonKeyword(pobj, JTOK_Null, "null", 3);
        case '/':
            if (pobj->options&0x01) {
                if (pobj->left-- <= 0)
                    return JTOK_Error;
                ch = *pobj->pos++;
                if (ch != '*' && ch != '/')
                    return JTOK_Error;

                if (ch == '*') {       /* slash,star comment */
                    while (pobj->left-- > 0) {
                        ch = *pobj->pos++;
                        if (ch == '*') {
                            if (pobj->left-- <= 0)
                                return JTOK_Error;
                            ch = *pobj->pos++;
                            if (ch == '/')
                                break;
                        }
                        if (ch == '\n')
                            pobj->line++;
                    }
                    if (pobj->left < 0)
                        return JTOK_Error; /* Error if not terminated */
                } else {               /* slash,slash comment */
                    while (pobj->left-- > 0) {
                        ch = *pobj->pos++;
                        if (ch == '\n')
                            pobj->line++;
                        if (ch == '\r' || ch == '\n')
                            break;
                    }
                }
            } else {
                return JTOK_Error;     /* Comment not allowed */
            }
            break;
        default:
            return JTOK_Error;
        }
    }
    return JTOK_End;
}


/* Match a keyword */
static int jsonKeyword(IoTP_json_parse_t * pobj, int otype, const char * match, int len) {
    if (pobj->left >= len) {
        if (!memcmp(pobj->pos, match+1, len)) {
            pobj->pos += len;
            pobj->left -= len;
            return otype;
        }
    }
    return JTOK_Error;
}


/* Return the value of a hex digit or -1 if not a valid hex digit */
static int hexValue(char ch) {
    if (ch <= '9' && ch >= '0')
        return ch-'0';
    if (ch >='A' && ch <= 'F')
        return ch-'A'+10;
    if (ch >='a' && ch <= 'f')
        return ch-'a'+10;
    return -1;
}


/* Match a string */
static int jsonString(IoTP_json_parse_t * pobj) {
    char * ip = pobj->pos;
    char * op = ip;
    int    left = pobj->left;
    int    i;
    int    val;
    int    digit;
    int    needcheck = 0;

    while (left > 0) {
        char ch = *ip++;
        if (ch == '"') {
            *op = 0;
            /*
             * If we have any suspicious characters, check for valid UTF-8.
             */
            if (needcheck) {
                if (validUTF8(pobj->pos, op-pobj->pos) < 0) {
                    pobj->rc = 2;
                    return JTOK_Error;
                }
            }
            pobj->pos = ip;
            pobj->left = left-1;
            return JTOK_String;
        } else if (ch=='\\') {
            if (left < 1)
                return JTOK_Error;
            ch = *ip++;
            left--;
            switch (ch) {
            case 'b':   ch = 0x08;   break;
            case 'f':   ch = 0x0c;   break;
            case 'n':   ch = '\n';   break;
            case 'r':   ch = '\r';   break;
            case 't':   ch = '\t';   break;

            case 'u':
                if (left < 4)
                    return JTOK_Error;
                /* Parse the hex value.  This must be four hex digits */
                val = 0;
                for (i=0; i<4; i++) {
                    digit = hexValue(*ip++);
                    if (digit < 0)
                        return JTOK_Error;
                    val = val<<4 | digit;
                }
                left -= 4;
                /* Do the UTF-8 expansion */
                if (val <= 0x7f) {
                    ch    = (char)val;
                } else if (val > 0x7ff) {
                    *op++ = (char)(0xe0 | ((val>>12) & 0x0f));
                    *op++ = (char)(0x80 | ((val>>6)  & 0x3f));
                    ch    = (char)(0x80 | (val & 0x3f));
                } else {
                    *op++ = (char)(0xc0 | ((val>>6) &0x1f));
                    ch    = (char)(0x80 | (val & 0x3f));
                }
                break;

            case '/':
            case '"':
            case '\\':
                break;
            default:
                return JTOK_Error;
            }
			*op++ = ch;
        } else{
            *op++ = ch;
            if ((signed char)ch < 0x20) {   /* C0 or > 0x7F */
                if ((uint8_t)ch < 0x20) {
                    pobj->rc = 2;
                    return JTOK_Error;      /* Control characters are not allowed in a string except when escaped */
                } else {
                    needcheck = 1;
                }
            }
        }
        left--;
    }
    return JTOK_Error;
}


/* Match a number.  */
static int jsonNumber(IoTP_json_parse_t * pobj) {
    char * dp = pobj->pos-2;
    int    left = pobj->left+1;
    int    state   = dp[1]=='-' ? 0 : 1;
    int    waszero = 0;
    int    digits  = 0;

    while (left > 0) {
        char ch = dp[1];
        switch (ch) {
        case '0':
            if (state==1 && waszero)
               return JTOK_Error;
            waszero = 1;
            if (state == 4)
                state = 5;
            digits++;
            break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if (state == 1)
                state = 2;
            if (state == 4)
                state = 5;
            waszero = 0;
            digits++;
            break;

        case '.':
            if (state != 1 && state != 2)
                return JTOK_Error;
            state = 3;
            digits = 0;
            break;

        case '-':
            if (state != 0 && state != 4)
                return JTOK_Error;
            state++;
            break;

        case 'E':
        case 'e':
            if (state==0 || state>3)
                return JTOK_Error;
            state = 4;
            digits = 0;
            break;

        case '+':
            if (state != 4)
                return JTOK_Error;
            state = 5;
            digits = 0;
            break;
        default:
            *dp++ = 0;
            if ((state==3 && digits == 0) || (state==5 && digits == 0))
                return JTOK_Error;
            pobj->pos = dp;
            pobj->left = left;
            return state > 2 ? JTOK_Number : JTOK_Integer;
        }
        *dp++ = ch;
        left--;
    }
    *dp++ = 0;
    pobj->pos = dp;
    pobj->left = 0;
    return JTOK_Error;
}

/*
 * Get a field from a JSON object
 */
int iotp_json_get(IoTP_json_parse_t * pobj, int entnum, const char * name) {
    int maxent;

    if (entnum < 0 || entnum >= pobj->ent_count || pobj->ent[entnum].objtype != JSON_Object) {
        return -1;
    }
    /* Allow the entry to be directly sent */
    if ((uintptr_t)name < pobj->ent_count) {
    	return (int)(uintptr_t)name;
    }
    maxent = entnum + pobj->ent[entnum].count;
    entnum++;
    while (entnum <= maxent) {
        IoTP_json_entry_t * ent = pobj->ent+entnum;
        if (!strcmp(name, ent->name)) {
            return entnum;
        }
        if (ent->objtype == JSON_Object || ent->objtype == JSON_Array) {
            entnum += ent->count+1;
        } else {
            entnum++;
        }
    }
    return -1;
}


/*
 * Get a string from a JSON object
 */
char * iotp_json_getString(IoTP_json_parse_t * pobj, const char * name) {
    IoTP_json_entry_t * ent;
    int    entnum;

    entnum = iotp_json_get(pobj, 0, name);
    if (entnum < 0)
        return NULL;
    ent = pobj->ent+entnum;
    switch (ent->objtype) {
    case JSON_True:    return "true";
    case JSON_False:   return "false";
    case JSON_Null:    return "null";
    default:           return NULL;
    case JSON_Integer:
    case JSON_String:
    case JSON_Number:
        return (char *)ent->value;
    }
}


/* Get an integer from a JSON object */
int iotp_json_getInt(IoTP_json_parse_t * pobj, const char * name, int deflt) {
    IoTP_json_entry_t * ent;
    int    entnum;
    int    val;
    char * eos;

    entnum = iotp_json_get(pobj, 0, name);
    if (entnum < 0)
        return deflt;
    ent = pobj->ent+entnum;
    switch (ent->objtype) {
    case JSON_Integer: return ent->count;
    case JSON_True:    return 1;
    case JSON_False:   return 0;
    default:           return deflt;
    case JSON_String:
    case JSON_Number:
        val = (int)strtod(ent->value, &eos);
        while (*eos==' ' || *eos=='\t')
            eos++;
        if (*eos)
            return deflt;
        return val;
    }
}

/* Get an integer from a JSON object. */
int iotp_json_getInteger(IoTP_json_parse_t * pobj, const char * name, int deflt) {
    IoTP_json_entry_t * ent;
    int    entnum;
    int    val;
    double dval;

    entnum = iotp_json_get(pobj, 0, name);
    if (entnum < 0)
        return deflt;
    ent = pobj->ent+entnum;
    switch (ent->objtype) {
    	case JSON_Integer:
    		return ent->count;
    	case JSON_Number:
    	    dval = strtod(ent->value, NULL);
    	    val = (int)dval;
    	    if (dval == (double)val)
    	        return val;
    	    else
    	        return deflt;
    	default:
    		return deflt;
    }
}

/* Get a JSON number as a double */
double iotp_json_getNumber(IoTP_json_parse_t * pobj, const char * name, double deflt) {
    IoTP_json_entry_t * ent;
    int    entnum;

    entnum = iotp_json_get(pobj, 0, name);
    if (entnum < 0)
        return deflt;
    ent = pobj->ent+entnum;
    switch (ent->objtype) {
        case JSON_Integer:
            return (double)ent->count;
        case JSON_Number:
            return strtod(ent->value, NULL);
        default:
            return deflt;
    }
}

/* Get object value using position */
char * iotp_json_getAttr(IoTP_json_parse_t *json, int pos, char *name) {
    int jPos = iotp_json_get(json, pos, name);
    if ( jPos != -1 ) {
        IoTP_json_entry_t * tent = json->ent + jPos;
        return (char *)tent->value;
    }
    return NULL;
}


