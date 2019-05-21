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

/** @file iotp_config.h 
    @brief **Client Configuration Reference** exposes configuration parameters and functioons to create and manage IoTPConfig object. The IoTPConfig object is used to create IoTP clients.

*/


/* IoTP Client names - defined in iotp_config.c */
extern char * IoTPClient_names[];

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
 * The Watson IoT Platform (WIoTP) client configuration.
 */
#define IoTPConfig_identity_orgId                       "identity.orgId"
#define IoTPConfig_identity_typeId                      "identity.typeId"
#define IoTPConfig_identity_deviceId                    "identity.deviceId"
#define IoTPConfig_identity_appId                       "identity.appId"
#define IoTPConfig_auth_key                             "auth.key"
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


/**
 * IoTPConfig_create() API creates IoTP client configuration object using 
 * the configuration items defined in configuration file (in YAML format)
 * specified by configFileName parameter.
 *
 * @param config         - A pointer to an IoTPConfig handle.
 * @param configFileName - Configuration file path.
 *                         If NULL, the API will return an empty config object.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 * @remark Use IoTPConfig_clear() API, to clear all properties.
 */
DLLExport IOTPRC IoTPConfig_create(IoTPConfig **config, const char *configFileName);

/**
 * IoTPConfig_readConfigFile() API updates the property settings in the config object from a YAML file.
 *
 * @param config         - A pointer to an IoTPConfig handle.
 * @param configFileName - Configuration file path
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPConfig_readConfigFile(IoTPConfig * config, const char * configFileName);

/**
 * IoTPConfig_readEnvironment() API updates the property settings from environment variables.
 *
 * @param config         - A pointer to an IoTPConfig handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPConfig_readEnvironment(IoTPConfig *config);

/**
 * IoTPConfig_setProperty() API updates the settings of an individual property.
 *
 * @param config         - A pointer to an IoTPConfig handle.
 * @param name           - Name of the property
 * @param value          - Value of the property
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPConfig_setProperty(IoTPConfig * config, const char * name, const char * value);

/**
 * IoTPConfig_clear() API clears config object, previously initialized using
 * IOTConfig_create() API.
 * 
 * @param config         - A pointer to an IoTPConfig handle.
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPConfig_clear(IoTPConfig *config);

/*
 * IoTPConfig_getProperty() API returns the settings of an individual property.
 *
 * @param config         - A pointer to an IoTPConfig handle.
 * @param name           - Name of the property
 * @param value          - Buffer to return property value
 * @param len            - Length of value buffer
 * @return IOTPRC        - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPConfig_getProperty(IoTPConfig *config, const char * name, char ** value, int len);


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
 * IoTPConfig_setLogHandler: Sets a Log Handler
 *
 * @param type           - Type of handler (IoTPLogHanlderType)
 * @param handler        - Pointer to Log handler
 * @return IOTPRC        - IOTPRC_SUCCESS for success or IOTPRC_*
 */
DLLExport IOTPRC IoTPConfig_setLogHandler(IoTPLogTypes type, void * handler);

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_CONFIG_H_ */
