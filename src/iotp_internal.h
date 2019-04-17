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


/* IoTP Handler - used for commands, events, notification, monitoring messages and DM Actions */
/* Start with DM actions types - match these with IoTP_DMHandler_Type_t defined in utils.h    */
typedef enum {
    IoTP_Handler_DMResponse          = 1,
    IoTP_Handler_DMUpdate            = 2,
    IoTP_Handler_DMObserve           = 3,
    IoTP_Handler_DMCancel            = 4,
    IoTP_Handler_DMFactoryReset      = 5,
    IoTP_Handler_DMReboot            = 6,
    IoTP_Handler_DMFirmwareDownload  = 7,
    IoTP_Handler_DMFirmwareUpdate    = 8,
    IoTP_Handler_DMActions           = 9,
    IoTP_Handler_Commands            = 10,
    IoTP_Handler_Command             = 11,
    IoTP_Handler_Notification        = 12,
    IoTP_Handler_MonitoringMessage   = 13,
    IoTP_Handler_DeviceMonitoring    = 14,
    IoTP_Handler_AppEvent            = 15,
    IoTP_Handler_AppMonitoring       = 16
} IoTP_Handler_type_t;

typedef struct {
    int             type;            /* IoTP_Handler_type_t                  */
    char *          topic;           /* Subscription topic                   */
    void *          cbFunc;          /* Callback function pointer            */
} IoTPHandler;

/* Callback Handlers */
typedef struct {
    IoTPHandler ** entries;          /* Array of callback handlers           */
    int            id;
    int            count;
    int            nalloc;
    int            slots;
    int            allCommandsId;    /* A callback for all commands is set   */
    int            allDMActionsId;   /* A callback for all DM acrions is set */
} IoTPHandlers;

/* Managed Client information */
typedef struct {
    char * serialNumber;
    char * manufacturer;
    char * model;
    char * deviceClass;
    char * description;
    char * fwVersion;
    char * hwVersion;
    char * descriptiveLocation;
} IoTPClientInfo;

/* Managed Client location */
typedef struct {
    double latitude;
    double longitude;
    double elevation;
    time_t measuredDateTime;
    double accuracy;
} IoTPClientLocation;

/* Managed Client firmware information */
typedef struct {
    char * version;
    char * name;
    char * url;
    char * verifier;
    int    state;
    int    updateStatus;
    char * deviceId;
    char * typeId;
    char * updatedDateTime;
} IoTPClientFirmware;

/* Managed Client action */
typedef struct {
    int    status;
    char * message;
    char * typeId;
    char * deviceId;
} IoTPClientAction;

/* Managed Client */
typedef struct IoTPManagedClient {
    int                lifetime;
    int                supportsDeviceActions;
    int                supportsFirmwareActions;
    int                observ;
    char *             metadata;
    char *             deviceInfo; 
    IoTPClientLocation deviceLocation;
    IoTPClientFirmware deviceFirmware;
    IoTPClientAction   deviceAction;
    char *             reqID;
    int                rc;
} IoTPManagedClient;

/* Strcture for IoTP client object */
typedef struct IoTPClient {
    int                 inited;
    IoTPClientType      type;
    void              * config;
    char              * clientId;
    char              * connectionURI;
    void              * mqttClient;
    IoTPHandlers      * handlers;
    int                 connected;
    int                 managed;
    IoTPManagedClient * managedClient;
} IoTPClient;

/* Device/gateway command topics */
#define COMMAND_ROOTTOPIC       "iot-2/"
#define COMMAND_ROOTTOPIC_LEN   6

/* Device Management topics */
/* Topics used for device to platform communications */
#define DM_MANAGE               "iotdevice-1/mgmt/manage"
#define DM_UNMANAGE             "iotdevice-1/mgmt/unmanage"
#define DM_NOTIFY               "iotdevice-1/notify"
#define DM_RESPONSE             "iotdevice-1/response"
#define DM_UPDATE_LOCATION      "iotdevice-1/device/update/location"
#define DM_CREATE_DIAG_ERRCODES "iotdevice-1/add/diag/errorCodes"
#define DM_CLEAR_DIAG_ERRCODES  "iotdevice-1/clear/diag/errorCodes"
#define DM_ADD_DIAG_LOG         "iotdevice-1/add/diag/log"
#define DM_CLEAR_DIAG_LOG       "iotdevice-1/clear/diag/log"

/* Topics used for platform to device communications */
#define DM_ACTION_RESPONSE          "iotdm-1/response"
#define DM_ACTION_UPDATE            "iotdm-1/device/update"
#define DM_ACTION_OBSERVE           "iotdm-1/observe"
#define DM_ACTION_CANCEL            "iotdm-1/cancel"
#define DM_ACTION_FACTORYRESET      "iotdm-1/mgmt/initiate/device/factory_reset"
#define DM_ACTION_REBOOT            "iotdm-1/mgmt/initiate/device/reboot"
#define DM_ACTION_FIRMWAREDOWNLOAD  "iotdm-1/mgmt/initiate/firmware/download"
#define DM_ACTION_FIRMWAREUPDATE    "iotdm-1/mgmt/initiate/firmware/update"
#define DM_ACTION_ALL               "iotdm-1/#"

#define DM_ACTION_ROOTTOPIC         "iotdm-1/"
#define DM_ACTION_ROOTTOPIC_LEN     8

/* Device action response code */
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

/* DM Action state */
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


/* Prototype of internal functions */
DLLExport IOTPRC iotp_client_create(void **client, IoTPConfig *config, IoTPClientType type);
DLLExport IOTPRC iotp_client_destroy(void *client, int destroyMQTTClient);
DLLExport IOTPRC iotp_client_connect(void *client);
DLLExport IOTPRC iotp_client_disconnect(void *client);
DLLExport IOTPRC iotp_client_setHandler(void *client, char * topic, int type, IoTPCallbackHandler handler);
DLLExport IOTPRC iotp_client_subscribe(void *client, char *topic, int qos);
DLLExport IOTPRC iotp_client_unsubscribe(void *client, char *topic);
DLLExport IOTPRC iotp_client_publish(void *client, char *topic, char *payload, int qos, MQTTProperties *props);
DLLExport IOTPRC iotp_client_retry_connection(void *client);
DLLExport IOTPRC iotp_client_isConnected(void *client);
DLLExport IOTPRC iotp_client_setMQTTLogHandler(void *client, IoTPLogHandler *cb);
DLLExport IOTPRC iotp_client_manage(void * client);
DLLExport IOTPRC iotp_client_unmanage(void * client, char *reqId);
DLLExport IOTPRC iotp_client_setAttribute(void *client, char *name, char *value);


/*
/// @endcond
*/

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_INTERNAL_H_ */
