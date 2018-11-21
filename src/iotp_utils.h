/*******************************************************************************
 * Copyright (c) 2018 IBM Corp.
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
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

#include "iotp_rc.h"
#include "iotp_config.h"

extern FILE *logger;

/*
/// @endcond
*/

/*! \page iotputils IoTP C Client Utilities

   TODO: Add details

 */

/**
 * IoTP Client log levels.
 * LOGLEVEL_TRACE will cause all trace entries  at all levels to be returned.
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
    LOGLEVEL_DEBUG   = 4,
    /** Low level trace message */
    LOGLEVEL_TRACE   = 5
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
 * IoTPCallbackHandler: Handler to process callbacks
 *
 * @param type           - IoTP client type ID
 *
 * @param id             - IoTP client ID
 *
 * @param callbackId     - Callback ID
 *
 * @param format         - Reponse format (e.g. JSON)
 *
 * @param payload        - Pointer to payload buffer
 *
 * @param payloadlen     - Size of payload buffer
 *
 */
typedef void (*IoTPCallbackHandler)(char* type, char* id, char* command, char *format, void* payload, size_t payloadlen);


/**
 * IoTPLogHandler: Handler to process log and trace messages from IoTP Client
 *
 * @param logLevel       - Log Level
 *
 * @param message        - Log or trace message
 *
 */
typedef void IoTPLogHandler(int logLevel, char *message);


/**
 * MQTTLogHandler: Handler to process log and trace messages from MQTT library
 *
 * @param logLevel       - Trace Level
 *
 * @param message        - Trace message
 *
 */
typedef void MQTTTraceHandler(int traceLevel, char *message);


/*
/// @cond EXCLUDE
*/

DLLExport void iotp_utils_logInvoke(IoTPLogLevel level, const char * func, const char * file, int line, const char * fmts, ...);
DLLExport void iotp_utils_setLogLevel(IoTPLogLevel level);
DLLExport void iotp_utils_freePtr(void * p);
DLLExport char * iotp_utils_trim(char *str);
DLLExport void iotp_utils_generateUUID(char* uuid_str);
DLLExport void iotp_utils_delay(long milsecs);
DLLExport void iotp_utils_writeClientVersion(void);
DLLExport IoTP_RC iotp_utils_setLogHandler(IoTPLogHandlerType type, void * handler);

#define LOG(sev, fmts...) \
        iotp_utils_logInvoke((LOGLEVEL_##sev), __FUNCTION__, __FILE__, __LINE__, fmts);

/*
/// @endcond
*/

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_UTILS_H_ */
