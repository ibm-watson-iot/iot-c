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
 * Contributors:
 *    Ranjan Dasgupt          - Initial API implementation
 *
 *******************************************************************************/

#if !defined(IOTP_UTILS_H_)
#define IOTP_UTILS_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/*
/// @cond EXCLUDE
*/

#if defined(WIN32) || defined(WIN64)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#else
  #define DLLImport extern
  #define DLLExport __attribute__ ((visibility ("default")))
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

#if defined(OSX)
#include <dlfcn.h>
#endif

#include "iotp_rc.h"


/*
 * Define standard integer types.
 */
#if defined(WIN32) || defined(WIN64)
#include <stddef.h>

#ifndef __INT8_T_DEFINED
#define __INT8_T_DEFINED
typedef signed char int8_t;
#endif

#ifndef __INT16_T_DEFINED
#define __INT16_T_DEFINED
typedef short int int16_t;
#endif

#ifndef __INT32_T_DEFINED
#define __INT32_T_DEFINED
typedef int int32_t;
#endif

#ifndef __INT64_T_DEFINED
#define __INT64_T_DEFINED
typedef __int64 int64_t;
#endif

#ifndef __UINT8_T_DEFINED
#define __UINT8_T_DEFINED
typedef unsigned char uint8_t;
#endif

#ifndef __UINT16_T_DEFINED
#define __UINT16_T_DEFINED
typedef unsigned short int uint16_t;
#endif

#ifndef __UINT32_T_DEFINED
#define __UINT32_T_DEFINED
typedef unsigned int uint32_t;
#endif

#ifndef __UINT64_T_DEFINED
#define __UINT64_T_DEFINED
typedef unsigned __int64 uint64_t;
#endif

#else

#include <inttypes.h>

#endif

#ifndef __ULL_T_DEFINED
#define __ULL_T_DEFINED
typedef unsigned long long ULL;
#endif

#define MAX_YAML_CONFIG_SECTIONS 5

/* Environment variables */
extern char **environ;

/* IoTP Clients logger handle */
extern FILE *logger;

/*
/// @endcond
*/

/**
 * IoTP Client log levels.
 * LOGLEVEL_DEBUG will cause all trace entries  at all levels to be returned.
 * LOGLEVEL_INFO will cause INFO, WARN and ERROR trace entries to be returned.
 */
typedef enum IoTPLogLevel {
    /** Error log message */
    LOGLEVEL_ERROR   = 1,
    /** Warning log message */
    LOGLEVEL_WARN    = 2,
    /** Informational message */
    LOGLEVEL_INFO    = 3,
    /** Debug message */
    LOGLEVEL_DEBUG   = 4
} IoTPLogLevel;

/**
 * Provides three qualities of service for delivering messages between client and server.
 */
typedef enum QoS {
    /** At most once   */
    QoS0  = 0,
    /** At least once  */
    QoS1  = 1,
    /** Extractly once */
    QoS2  = 2
} QoS;

/**
 *  List of Log handler types
 */
typedef enum IoTPLogTypes {
    /** Callback function */
    IoTPLog_Callback = 1,

    /** File pointer */
    IoTPLog_FilePointer = 2,

    /** File descriptor */
    IoTPLog_FileDescriptor = 3

} IoTPLogTypes;

/**
 * List of Device Management actions and response that can be invoked by the platform.
 */
typedef enum {
    IoTP_DMResponse          = 1,
    IoTP_DMUpdate            = 2,
    IoTP_DMObserve           = 3,
    IoTP_DMCancel            = 4,
    IoTP_DMFactoryReset      = 5,
    IoTP_DMReboot            = 6,
    IoTP_DMFirmwareDownload  = 7,
    IoTP_DMFirmwareUpdate    = 8,
    IoTP_DMActions           = 9
} IoTP_DMAction_type_t;

/**
 * List of device management action response codes 
 */
#define DM_ACTION_RC_REBOOT_INITIATED            202
#define DM_ACTION_RC_REBOOT_FAILED               500
#define DM_ACTION_RC_REBOOT_NOTSUPPORTED         501
#define DM_ACTION_RC_FACTORYRESET_INITIATED      202
#define DM_ACTION_RC_FACTORYRESET_FAILED         500
#define DM_ACTION_RC_FACTORYRESET_NOTSUPPORTED   501
#define DM_ACTION_RC_UPDATE_SUCCESS              204
#define DM_ACTION_RC_RESPONSE_SUCCESS            200
#define DM_ACTION_RC_RESPONSE_ACCEPTED           202
#define DM_ACTION_RC_BAD_REQUEST                 400
#define DM_ACTION_RC_UPDATE_SUCCESS              204

/**
 * List of device management action states 
 */
#define FIRMWARESTATE_IDLE                 0
#define FIRMWARESTATE_DOWNLOADING          1
#define FIRMWARESTATE_DOWNLOADED           2
#define FIRMWAREUPDATE_SUCCESS             0
#define FIRMWAREUPDATE_INPROGRESS          1
#define FIRMWAREUPDATE_OUTOFMEMORY         2
#define FIRMWAREUPDATE_CONNECTIONLOST      3
#define FIRMWAREUPDATE_VERIFICATIONFAILED  4
#define FIRMWAREUPDATE_UNSUPPORTEDIMAGE    5
#define FIRMWAREUPDATE_INVALIDURL          6

/*
/// @cond EXCLUDE
*/

/*
 * JSON entry object
 */
typedef struct IoTP_json_entry_t {
    int    objtype;        /* JSON entry type                               */
    int    count;          /* Count for object and array, value for integer */
    int    level;          /* Level of JSON entry in the tree               */
    int    line;           /* The line number in the source                 */
    const char * name;     /* utf-8 entry name when in an object            */
    const char * value;    /* utf-8 entry value for string and number       */
} IoTP_json_entry_t;

/*
 * JSON parse object.
 */
typedef struct IoTP_json_parse_t {
    IoTP_json_entry_t *  ent;            /* Entry array         */
    char *              source;         /* Source string       */
    int                 src_len;        /* Source length       */
    int                 ent_alloc;      /* Allocated entries   */
    int                 ent_count;      /* Used entries        */
    uint8_t             free_source;    /* Source should be freed when done */
    uint8_t             free_ent;       /* Entry array should be freed when done */
    uint8_t             options;        /* 0x01=allow comments */
    uint8_t             resv;
    int                 rc;             /* Return code from parse */
    int                 line;           /* Offset where error was found */
    char *              pos;            /* Internal use during parse */
    int                 left;           /* Internal use during parse */
    int                 resvi;
} IoTP_json_parse_t;

/*
 * JSON entry types
 */
enum IoTP_json_obj_type_e {
    JSON_String   = 1,   /* JSON string, value is UTF-8              */
    JSON_Integer  = 2,   /* Number with no decimal point             */
    JSON_Number   = 3,   /* Number which is too big or has a decimal */
    JSON_Object   = 4,   /* JSON object, count is number of entries  */
    JSON_Array    = 5,   /* JSON array, count is number of entries   */
    JSON_True     = 6,   /* JSON true, value is not set              */
    JSON_False    = 7,   /* JSON false, value is not set             */
    JSON_Null     = 8    /* JSON null, value is not set              */
};

/*
 * Tokens returned by the JSON tokenizer
 */
enum IoTP_json_token_e {
    JTOK_Error          = 1,
    JTOK_StartObject    = 2,
    JTOK_EndObject      = 3,
    JTOK_StartArray     = 4,
    JTOK_EndArray       = 5,
    JTOK_Colon          = 6,
    JTOK_Comma          = 7,
    JTOK_String         = 8,
    JTOK_Number         = 9,
    JTOK_Integer        = 10,
    JTOK_True           = 11,
    JTOK_False          = 12,
    JTOK_Null           = 13,
    JTOK_End            = 14,
};

/*
 * Parser states
 */
enum IoTP_json_state_e {
    JSTATE_Name,      /* In object, looking for name */
    JSTATE_Value,     /* In object or array, looking for value */
    JSTATE_Comma,     /* In object or array, looking for separator or terminator */
    JSTATE_Done,      /* Done processing due to end of message or error */
};

/*
/// @endcond
*/

/**
 * IoTPCallbackHandler: Handler to process callbacks
 *
 * @param type           - IoTP client (device/gateway) type ID
 * @param id             - IoTP client ID
 * @param command        - Command ID
 * @param format         - Reponse format (e.g. JSON)
 * @param payload        - Pointer to payload buffer
 * @param payloadlen     - Size of payload buffer
 */
typedef void (*IoTPCallbackHandler)(char* type, char* id, char* command, char *format, void* payload, size_t payloadlen);

/**
 * IoTPDMActionHandler: Handler to process device and firmware action Callback.
 * Platform sends payload in JSON format.
 *
 * @param type           - DM Action type - IoTP_DMAction_type_t
 * @param reqId          - Request ID
 * @param payload        - Pointer to payload buffer
 * @param payloadlen     - Size of payload buffer
 */
typedef void (*IoTPDMActionHandler)(IoTP_DMAction_type_t type, char *reqId, void *payload, size_t payloadlen);


/**
 * IoTPEventCallbakHandler: Handler to process Event Callback.
 *
 * @param type           - IoTP client ID
 * @param rc             - IOTPRC_SUCCESS or IOTPRC_FAILURE
 * @param success        - Callbak success response
 * @param failure        - Callbak failure response
 */
typedef void (*IoTPEventCallbackHandler)(char *id, int rc, void *success, void *failure);

/**
 * IoTPLogHandler: Callback handler to process log and trace messages from IoTP Client.
 *
 * @param logLevel       - Log Level
 * @param message        - Log or trace message
 */
typedef void IoTPLogHandler(int logLevel, char *message);

/**
 * MQTTLogHandler: Callback handler to process log and trace messages from MQTT library.
 *
 * @param logLevel       - Trace Level
 * @param message        - Trace message
 */
typedef void MQTTTraceHandler(int traceLevel, char *message);

/*
/// @cond EXCLUDE
*/

DLLExport void iotp_utils_log(IoTPLogLevel level, const char * file, const char * func, int line, const char * fmts, ...);
DLLExport void iotp_utils_setLogLevel(IoTPLogLevel level);
DLLExport void iotp_utils_freePtr(void * p);
DLLExport char * iotp_utils_trim(char *str);
DLLExport void iotp_utils_generateUUID(char* uuid_str);
DLLExport void iotp_utils_delay(long milsecs);
DLLExport void iotp_utils_writeClientVersion(void);
DLLExport IOTPRC iotp_utils_setLogHandler(IoTPLogTypes type, void * handler);
DLLExport IOTPRC iotp_utils_fileExist(const char * filePath);
DLLExport char * iotp_utils_getToken(char * from, const char * leading, const char * trailing, char * * more);
DLLExport IoTP_json_parse_t * iotp_json_init(int payloadlen, char *payload);
DLLExport int iotp_json_free(IoTP_json_parse_t * pobj); 
DLLExport int iotp_json_parse(IoTP_json_parse_t * pobj);
DLLExport int iotp_json_get(IoTP_json_parse_t * pobj, int entnum, const char * name);
DLLExport char * iotp_json_getString(IoTP_json_parse_t * pobj, const char * name);
DLLExport int iotp_json_getInt(IoTP_json_parse_t * pobj, const char * name, int deflt);
DLLExport int iotp_json_getInteger(IoTP_json_parse_t * pobj, const char * name, int deflt);
DLLExport double iotp_json_getNumber(IoTP_json_parse_t * pobj, const char * name, double deflt);
DLLExport char * iotp_json_getAttr(IoTP_json_parse_t * pobj, int pos, char * name);
DLLExport int iotp_match_mqttTopic(const char * topic, const char * filter);

#define LOG(sev, fmts...) iotp_utils_log((LOGLEVEL_##sev), __FILE__, __FUNCTION__, __LINE__, fmts);

/*
/// @endcond
*/

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_UTILS_H_ */
