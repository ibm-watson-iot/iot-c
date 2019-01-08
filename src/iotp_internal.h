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

#include <MQTTProperties.h>

#include "iotp_rc.h"
#include "iotp_config.h"
#include "iotp_utils.h"

/*
 * Optional configuration items for internal testing only.
 *
 * authMethod  The "authMethod" specifies the type of authentication method used
 *             for connecting to WIoTP service. The valid values are token, cert.
 *             By default, this parameter is not set. Client authentication will be
 *             based on the settings of token, certificatePath, and Device.keyPath.
 *             This parameter can be used to force a client to authenticate using
 *             token or certificate. This parameter is mainly used for testing.
 */
#define IoTPConfig_options_authMethod                   "options.authMethod"


/* IoTP identity configuration items */
typedef struct identity_t {
    char * orgId;
    char * typeId;
    char * deviceId;
    char * appId;
} identity_t;

/* IoTP auth configuration items */
typedef struct auth_t {
    char * apiKey;
    char * token;
    char * keyStore;
    char * privateKey;
    char * privateKeyPassword;
} auth_t;

/* Optional MQTT configuration items */
typedef struct mqttopts_t {
    char * transport;
    char * caFile;
    int    validateServerCert;
    int    port;
    int    traceLevel;
    int    cleanSession;
    int    cleanStart;
    int    keepalive;
    int    sessionExpiry;
    int    sharedSubscription;
} mqttopts_t;



#ifdef HTTP_IMPLEMENTED

/* Optional HTTP configuration items */
typedef struct httpopts_t {
    int    validateServerCert;
    char * caFile;
} httpopts_t;

#endif



/* IoTP client config object - includes optional items */
typedef struct IoTPConfig {
    char           * domain;
    IoTPLogLevel     logLevel;
    IoTPClientType   type;
    identity_t     * identity;
    auth_t         * auth;
    mqttopts_t     * mqttopts;
#ifdef HTTP_IMPLEMENTED
    httpopts_t     * httpopts;
#endif
    int              authMethod;            /* for internal used only */
    int              automaticReconnect;    /* for internal use only */
} IoTPConfig;

/*
 * NOTE:
 * The following optional configuration items are added for internal testing only.
 *
 * authMethod  The "authMethod" specifies the type of authentication method used
 *             for connecting to WIoTP service. The valid values are token, cert.
 *             By default, this parameter is not set. Client authentication will be
 *             based on the settings of token, certificatePath, and Device.keyPath.
 *             This parameter can be used to force a client to authenticate using
 *             token or certificate. This parameter is mainly used for testing.
 * automaticReconnect   Setting "automaticReconnect" to 1 will make the client
 *                      to reconnect if connection to the server is broken.
 */
#define IoTPInternal_options_authMethod                   "options.authMethod"
#define IoTPInternal_options_automaticReconnect           "options.automaticReconnect"


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
    int                 connected;
} IoTPClient;

DLLExport IoTP_RC iotp_client_create(void **client, IoTPConfig *config, IoTPClientType type);
DLLExport IoTP_RC iotp_client_destroy(void *client, int destroyMQTTClient);
DLLExport IoTP_RC iotp_client_connect(void *client);
DLLExport IoTP_RC iotp_client_disconnect(void *client);
DLLExport IoTP_RC iotp_client_setHandler(void *client, char * topic, int type, IoTPCallbackHandler handler);
DLLExport IoTP_RC iotp_client_subscribe(void *client, char *topic, int qos);
DLLExport IoTP_RC iotp_client_unsubscribe(void *client, char *topic);
DLLExport IoTP_RC iotp_client_publish(void *client, char *topic, char *payload, int qos, MQTTProperties *props);
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
