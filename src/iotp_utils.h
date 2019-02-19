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

#define MAX_YAML_CONFIG_SECTIONS 5

/* Environment variables */
extern char **environ;

/* IoTP Clients logger handle */
extern FILE *logger;

/*
/// @endcond
*/

/*! \page iotputils IoTP C Client Utilities

   TODO: Add details

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
 * IoTPCallbackHandler: Handler to process callbacks
 *
 * @param type           - IoTP client (device/gateway) type ID
 * @param id             - IoTP client ID
 * @param callbackId     - Callback ID e.g. commandId
 * @param format         - Reponse format (e.g. JSON)
 * @param payload        - Pointer to payload buffer
 * @param payloadlen     - Size of payload buffer
 */
typedef void (*IoTPCallbackHandler)(char* type, char* id, char* command, char *format, void* payload, size_t payloadlen);

/**
 * List of Device Management actions and response that can be the platform.
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

#define LOG(sev, fmts...) iotp_utils_log((LOGLEVEL_##sev), __FILE__, __FUNCTION__, __LINE__, fmts);

/*
/// @endcond
*/

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_UTILS_H_ */
