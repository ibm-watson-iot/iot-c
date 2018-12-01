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

#if !defined(IOTP_INTERNAL_H_)
#define IOTP_INTERNAL_H_

#if defined(__cplusplus)
 extern "C" {
#endif


/*
 * Defines structures and functions for internal use.
 * Do not use these in Watson IoT Platform clients: 
 *    device, gateway, application, managed (device and gateway)
 */

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
#include "iotp_utils.h"

/* IoTP client */
typedef struct iotc {
        char * typeId;
        char * deviceId;
        int    authMethod;
        char * authToken;
        char * certificatePath;
        char * keyPath;
} iotc;
/* IoTP application */
typedef struct iota {
        char * appId;
        char * authToken;
        char * APIKey;
} iota;
/* IoTP config object */
typedef struct IoTPConfig {
    char * orgId;
    char * domain;
    char * serverCertificatePath;
    int    port;
    int    logLevel;
    int    MQTTTraceLevel;
    iotc * device;
    iotc * gateway;
    iota * application;
} IoTPConfig;


/* IoTP Handler - used for commands, events, notification, monitoring messages */
typedef enum {
    IoTP_Handler_GlobalCommand     = 1,
    IoTP_Handler_Command           = 2,
    IoTP_Handler_Notification      = 3,
    IoTP_Handler_MonitoringMessage = 4,
    IoTP_Handler_Event             = 5,
    IoTP_Handler_DeviceMonitoring  = 6,
    IoTP_Handler_AppMonitoring     = 7
} IoTP_Handler_type_t;

typedef struct {
    int                 type;         /* IoTP_Handler_type_t       */
    char *              topic;        /* Subscription topic        */
    void *              cbFunc;       /* Callback function pointer */
} IoTPHandler;

/* Command Handlers */
typedef struct {
    IoTPHandler      ** entries;
    int                 id;
    int                 count;
    int                 nalloc;
    int                 slots;
} IoTPHandlers;

/* Strcture for IoTP client object */
typedef struct IoTPClient {
    int                 inited;
    IoTPClientType      type;
    void *              config;
    char *              clientId;
    char *              connectionURI;
    void *              mqttClient;
    IoTPHandlers *      handlers;
    int                 keepAliveInterval;
    int                 connected;
} IoTPClient;

DLLExport IoTP_RC iotp_client_create(void **client, IoTPConfig *config, IoTPClientType type);
DLLExport IoTP_RC iotp_client_destroy(void *client, int destroyMQTTClient);
DLLExport IoTP_RC iotp_client_connect(void *client);
DLLExport IoTP_RC iotp_client_disconnect(void *client);
DLLExport IoTP_RC iotp_client_setHandler(void *client, char * topic, int type, IoTPCallbackHandler handler);
DLLExport IoTP_RC iotp_client_subscribe(void *client, char *topic, int qos);
DLLExport IoTP_RC iotp_client_unsubscribe(void *client, char *topic);
DLLExport IoTP_RC iotp_client_publish(void *client, char *topic, char *payload, int qos);
DLLExport IoTP_RC iotp_client_retry_connection(void *client);
DLLExport IoTP_RC iotp_client_isConnected(void *client);
DLLExport IoTP_RC iotp_client_setMQTTLogHandler(void *client, IoTPLogHandler *cb);


/*
/// @endcond
*/

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_INTERNAL_H_ */
