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

#if !defined(IOTP_CONFIG_H_)
#define IOTP_CONFIG_H_

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

#include <stdio.h>
#include "iotp_rc.h"
#include "iotp_utils.h"

/*
/// @endcond
*/


/*! \page iotpconfig IoTP C Client Configuration
  
   IoTP Config object defines the configuration parameters to configure IoTP client libraries for:
     - Device
     - Gateway 
     - Application
     - Managed device
     - Managed gateway

   Configuration is portable between different client libraries.
  
   IoTP Config object can be created from configuration parameters defined in an YAML file, or 
   as environment variables. Users can also use API to set a specific configuration parameter.
  
   The configuration parameters are grouped into the following sections:
     - Identity: Configuration items related to identity of the client.
     - Auth: Configuration items related to authentication.
     - Options: Optional configuration items.
 
   Sample YAML file to configure a device: <br>
   <pre>
       identity:
         orgId: xxxxxx
         deviceType: devTypeA
         deviceId: devA
       auth:
         token: xxxxxxxx
       options:
         logLevel: debug
         mqtt:
           port: 443
   </pre>

   The configuration items may be groups into a high level section based on the type of client:
     - Device: Configuration items to configure a device client.
     - Gateway: Configuration items to configure a gateway client.
     - Application: Configuration items to configure an application client.

   This option can be used to define configuration of multiple clients in the same configuration
   file. Example of a device configuration: <br>
   <pre>
       device:
         identity:
           orgId: xxxxxx
           deviceType: devTypeA
           deviceId: devA
         auth:
           token: xxxxxxxx
         options:
           logLevel: debug
           mqtt:
             port: 443
   </pre>
  
   To set IoTPConfig objects using environment variable, use the following format: <br>
       iotp.configuration_category_name.configuration_parameter_name
  
       Example: <br>
       <pre>
           wiotp.identiry.orgid=xxxxxx
           wiotp.auth.token=xxxxxxxx
           wiotp.options.mqtt.port=443
       </pre>
       It can also be defined as:
       <pre>
           wiotp.device.identiry.orgid=xxxxxx
           wiotp.device.auth.token=xxxxxxxx
           wiotp.device.options.mqtt.port=443
       </pre>
   
 */

/* IoTP Client names - defined in iotp_config.c */
extern char * IoTPClient_names[];

/**
 * IoTPLogHandler: Callback to process log and trace messages from IoTP Client
 *
 * @param logLevel       - Log Level
 *
 * @param message        - Log or trace message
 *
 */
typedef void IoTPLogHandler(int logLevel, char *message);


/**
 * A handle representing an IBM Watson IoTP client configuration object.
 * A valid configuration object handle is available following a successful 
 * call to IoTPConfig_create().
 */
typedef struct IoTPConfig IoTPConfig;


/**
 *  List of Watson IoT Platform clients 
 */
typedef enum {
    /** Device client */
    IoTPClient_device = 1,

    /** Gateway client */
    IoTPClient_gateway = 2,

    /** Application client - small a */
    IoTPClient_application = 3,

    /** Scalable application client - big A */
    IoTPClient_Application = 4,

    /** Managed device client */
    IoTPClient_managed_device = 5,

    /** Managed gateway client */
    IoTPClient_managed_gateway = 6,

    /* Number of client types - should be the last entry in this enum */
    IoTPClient_total = 7 

} IoTPClientType;


/**
 * The Watson IoT Platform (WIoTP) client configuration items are grouped into three categories: <br>
 *
 * 1. Identity: Required items to establish unique identity of the client in WIoTP service. <br>
 * 2. Auth:     Required items to get authenticated and authorizerd with WIoTP service. <bt>
 * 3. Options:  Optional items required for interacting with WIoTP service. <br>
 *
 * Identity configuration item for a device or gateway client: <br>
 * orgId     The "orgId" is a unique six character identifier assigned to the users <br>
 *           when they register with WIoTP service. <br>
 * typeId    The "typeId" attribute represents the model of the device or gateway. <br>
 *           The "typeId" must be registered with WIoTP service. <br>
 * deviceId  The "deviceId" attribute identifies the device or the gateway, e.g. serial number. <br>
 *           The "deviceId" must be registered with WIoTP service. <br>
 *
 * Identity configuration item for an application client: <br>
 * appId     The "appId" is a unique identifier of an application in WIoTP organization. <br>
 *
 *
 * Authentication configuration item for a device or gateway client: <br>
 *
 * Authentication configuration item for an application client: <br>
 *
 * Optional configuration items: <br>
 * domain    The "domain" specifies the messaging endpoint URL. <br>
 *           The default value is "internetofthings.ibmcloud.com" <br>
 * logLevel  The "logLevel" specifies the debug logging level used by the client. <br>
 *           The valid values are "ERROR", "WARN", "INFO", and "DEBUG".
 *           The default value is "ERROR". <br>
 *
 * Optional configuration items for MQTT protocol: <br>
 * port      The "port" specifies the port number to connect to IBM Watson IoT Platform. <br>
 *           The valid values are 8883 and 443. The default value is 8883. 
 * caFile    The "caFile" specifies platform server certificate to varify host. <br>
 *           The default certificate "IoTPlatform.pem" is bundled with the client code. <br>
 *           The default value is "./IoTPlatform.pem"
 *
 * Optional configuration items for HTTP protocol: <br>
 *
 */
#define IoTPConfig_identity_orgId                       "identity.orgId"
#define IoTPConfig_identity_typeId                      "identity.typeId"
#define IoTPConfig_identity_deviceId                    "identity.deviceId"
#define IoTPConfig_identity_appId                       "identity.appId"
#define IoTPConfig_auth_APIKey                          "auth.APIKey"
#define IoTPConfig_auth_token                           "auth.token"
#define IoTPConfig_auth_keyStore                        "auth.keyStore"
#define IoTPConfig_auth_privateKey                      "auth.privateKey"
#define IoTPConfig_auth_privateKeyPassword              "auth.privateKeyPassword"
#define IoTPConfig_options_domain                       "options.domain"
#define IoTPConfig_options_logLevel                     "options.logLevel"
#define IoTPConfig_options_mqtt_traceLevel              "options.mqtt.traceLevel"
#define IoTPConfig_options_mqtt_transport               "options.mqtt.transport"
#define IoTPConfig_options_mqtt_caFile                  "options.mqtt.caFile"
#define IoTPConfig_options_mqtt_port                    "options.mqtt.port"
#define IoTPConfig_options_mqtt_cleanSession            "options.mqtt.cleanSession"
#define IoTPConfig_options_mqtt_cleanStart              "options.mqtt.cleanStart"
#define IoTPConfig_options_mqtt_sessionExpiry           "options.mqtt.sessionExpiry"
#define IoTPConfig_options_mqtt_keepalive               "options.mqtt.keepalive"
#define IoTPConfig_options_mqtt_sharedSubscription      "options.mqtt.sharedSubscription"
#define IoTPConfig_options_mqtt_validateServerCert      "options.mqtt.validateServerCert"

#ifdef HTTP_IMPLEMENTED
#define IoTPConfig_options_http_caFile                  "options.http.caFile"
#define IoTPConfig_options_http_validateServerCert      "options.http.validateServerCert"
#endif


/*
 * IoTPConfig_setLogHandler: Sets a Log Handler
 *
 * @param type           - Type of handler (IoTPLogHanlderType)
 *
 * @param handler        - Pointer to Log handler
 *
 * @return IoTP_RC       - IoTP_SUCCESS for success or IoTP_RC_*
 *
 */
DLLExport IoTP_RC IoTPConfig_setLogHandler(IoTPLogTypes type, void * handler);

/**
 * IoTPConfig_create: Creates IoTPConfig object.
 *
 * This function creates IBM Watson IoTP client configuration object.
 * 
 * @param config         - A pointer to an IoTPConfig handle.
 *
 * @param configFileName - Configuration file path.
 *                         If NULL, the API will return an empty config object.
 *
 * @return IoTP_RC  - Returns one of the following codes: 
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                       - IoTP_RC_INVALID_NOMEM  if system runs out of memory
 *                       - IoTP_RC_FILE_OPEN if unable to open log file
 *                       - IoTP_RC_INVALID_PARAM if a specified parameter is not valid
 *                       - IoTP_RC_MISSING_INPUT_PARAM if a required parameter is not specified
 *                       - IoTP_RC_QUICKSTART_NOT_SUPPORTED if a specified parameter is not valid for quitckstart
 *
 * Use IoTPConfig_clear() API, to clear all properties.
 *
 */
DLLExport IoTP_RC IoTPConfig_create(IoTPConfig **config, const char *configFileName);


/*
 * IoTPConfig_readConfigFile: Updates the property settings in the config object from a YAML file
 *
 * @param config         - A pointer to an IoTPConfig handle.
 *
 * @param configFileName - Configuration file path
 *
 * @return IoTP_RC  - Returns one of the following codes: 
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                       - IoTP_RC_INVALID_NOMEM if system runs out of memory
 *                       - IoTP_RC_FILE_OPEN if unable to open log file
 *                       - IoTP_RC_INVALID_PARAM if a specified parameter is not valid
 *                       - IoTP_RC_MISSING_INPUT_PARAM if a required parameter is not specified
 *                       - IoTP_RC_QUICKSTART_NOT_SUPPORTED if a specified parameter is not valid for quitckstart
 *
 */
DLLExport IoTP_RC IoTPConfig_readConfigFile(IoTPConfig * config, const char * configFileName);


/*
 * IoTPConfig_readEnvironment: Updates the property settings from environment variables.
 *
 * @param config         - A pointer to an IoTPConfig handle.
 *
 * @return IoTP_RC  - Returns one of the following codes: 
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                       - IoTP_RC_INVALID_NOMEM if system runs out of memory
 *                       - IoTP_RC_INVALID_PARAM if a specified parameter is not valid
 *                       - IoTP_RC_PARAM_INVALID_VALUE - Property value is not valid
 *                       - IoTP_RC_QUICKSTART_NOT_SUPPORTED if a specified parameter is not valid for quitckstart
 *
 */
DLLExport IoTP_RC IoTPConfig_readEnvironment(IoTPConfig *config);


/*
 * IoTPConfig_setProperty: Updates the settings of an individual property
 *
 * @param config         - A pointer to an IoTPConfig handle.
 *
 * @param name           - Name of the property
 *
 * @param value          - Value of the property
 *
 * @return IoTP_RC  - Returns one of the following codes: 
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                       - IoTP_RC_INVALID_NOMEM if system runs out of memory
 *                       - IoTP_RC_INVALID_PARAM if a specified parameter is not valid
 *                       - IoTP_RC_PARAM_INVALID_VALUE - Property value is not valid
 *                       - IoTP_RC_QUICKSTART_NOT_SUPPORTED if a specified parameter is not valid for quitckstart
 *
 */
DLLExport IoTP_RC IoTPConfig_setProperty(IoTPConfig * config, const char * name, const char * value);


/**
 * IoTPConfig_clear: Clear all properties.
 *
 * This function clears IBM Watson IoT Config object, previously initialized using
 * IOTConfig_create() API.
 * 
 * @param config         - A pointer to an IoTPConfig handle.
 *
 * @return IoTP_RC  - Returns one of the following codes: 
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *
 */
DLLExport IoTP_RC IoTPConfig_clear(IoTPConfig *config);


/*
 * IoTPConfig_getProperty: Returns the settings of an individual property
 *
 * @param config         - A pointer to an IoTPConfig handle.
 *
 * @param name           - Name of the property
 *
 * @param value          - Buffer to return property value
 *
 * @param len            - Length of value buffer
 *
 * @return IoTP_RC  - Returns one of the following codes: 
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                       - IoTP_RC_INVALID_PARAM if a specified parameter is not valid
 *
 */
DLLExport IoTP_RC IoTPConfig_getProperty(IoTPConfig *config, const char * name, char ** value, int len);


#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_CONFIG_H_ */
