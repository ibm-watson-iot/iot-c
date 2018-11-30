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
  
   The configuration parameters are grouped into the following catagories:
     - Platform: Configuration items related to WIoTP.
     - Organizarion: Configuration items related to WIoTP service subscribed by the user.
     - Device: Configuration items to configure a device.
     - Gateway: Configuration items to configure a gateway.
     - Application: Configuration items to configure an application.
     - Debug: Configuration items related to debug options.
  
   Sample YAML file to configure a device: <br>
   <pre>
       Organization:
         id: xxxxxx
       Device:
         typeId: SensorTypeA
         deviceId: Sensor0001
         authToken: xxxxxxxxxx
   </pre>
  
   To set IoTPConfig objects using environment variable, use the following format: <br>
       iotp.configuration_category_name.configuration_parameter_name
  
       Example: <br>
       <pre>
           iotp.organization.id=xxxxxx
           iotp.device.typeId=SensorTypeA
           iotp.device.deviceId=Sensor0001
           iotp.device.authToken=xxxxxxxxxx
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

    /* Number of client types */
    IoTPClient_total = 6 

} IoTPClientType;


/* List of configuration parameters to create IoTPConfig object */

/** 
 *  Platform.domain is an optional configuration parameter in "Platform" configuration category. <br>
 *  The "domain" specifies the messaging endpoint URL. <br>
 *  The default value is "internetofthings.ibmcloud.com" 
 */
#define IoTPConfig_Platform_domain    "Platform.domain"

/** 
 *  Platform.port is an optional configuration parameter in "Platform" configuration category. <br>
 *  The "port" specifies the por number to connect to IBM Watson IoT Platform. <br>
 *  The valid values are 8883 and 443. The default value is 8883 
 */
#define IoTPConfig_Platform_port    "Platform.port"

/**
 *  Platform.serverCertificatePath is an optional configuration parameter in "Platform" configuration category. <br>
 *  The "serverCertificatePath" specifies platform server certificate to varify host. <br>
 *  The default certificate "IoTPlatform.pem" is bundled with the client code. <br>
 *  The default value is "./IoTPlatform.pem"
 */
#define IoTPConfig_Platform_serverCertificatePath    "Platform.serverCertificatePath"

/**
 *  Organizarion.id is a required configuration parameter in "Organization" configuration category. <br>
 *  The "id" is a unique six character identifier assigned to the users when they register with
 *  the Watson IoT Platform.
 */
#define IoTPConfig_Organization_id    "Organization.id"

/**
 *  Device.typeId is a required configuration parameter in "Device" configuration category. <br>
 *  
 */
#define IoTPConfig_Device_typeId    "Device.typeId"

/**
 *  Device.deviceId is a required configuration parameter in "Device" configuration category. <br>
 *
 */
#define IoTPConfig_Device_deviceId    "Device.deviceId"

/**
 *  Device.authMethod is an optional configuration parameter in "Device" configuration category. <br>
 *  The "authMethod" specifies the type of authentication method used for connecting to IBM Watson IoT Platform. <br>
 *  The valid values are token, cert. <br>
 *  By default, this parameter is not set. Client authentication will be based on the 
 *  settings of Device.authToken, Device.certificatePath, and Device.keyPath
 */
#define IoTPConfig_Device_authMethod    "Device.authMethod"

/**
 *  Device.authToken is an optional configuration parameter in "Device" configuration category. <br>
 *  
 */
#define IoTPConfig_Device_authToken    "Device.authToken"

/**
 *  Device.certificatePath is an optional configuration parameter in "Device" configuration category. <br>
 * 
 */
#define IoTPConfig_Device_certificatePath    "Device.certificatePath"

/**
 *  Device.keyPath is an optional configuration parameter in "Device" configuration category. <br>
 *
 */
#define IoTPConfig_Device_keyPath    "Device.keyPath"


/**
 *  Gateway.typeId is a required configuration parameter in "Gateway" configuration category. <br>
 *  
 */
#define IoTPConfig_Gateway_typeId    "Gateway.typeId"

/**
 *  Gateway.deviceId is a required configuration parameter in "Gateway" configuration category. <br>
 *
 */
#define IoTPConfig_Gateway_deviceId    "Gateway.deviceId"

/**
 *  Gateway.authMethod is an optional configuration parameter in "Gateway" configuration category. <br>
 *  The "authMethod" specifies the type of authentication method used for connecting to IBM Watson IoT Platform. <br>
 *  The valid values are token, cert. <br>
 *  By default, this parameter is not set. Client authentication will be based on the 
 *  settings of Gateway.authToken, Gateway.certificatePath, and Gateway.keyPath
 */
#define IoTPConfig_Gateway_authMethod    "Gateway.authMethod"

/**
 *  Gateway.authToken is an optional configuration parameter in "Gateway" configuration category. <br>
 *  
 */
#define IoTPConfig_Gateway_authToken    "Gateway.authToken"

/**
 *  Gateway.certificatePath is an optional configuration parameter in "Gateway" configuration category. <br>
 * 
 */
#define IoTPConfig_Gateway_certificatePath    "Gateway.certificatePath"

/**
 *  Gateway.keyPath is an optional configuration parameter in "Gateway" configuration category. <br>
 *
 */
#define IoTPConfig_Gateway_keyPath    "Gateway.keyPath"


/**
 *  Application.appId is a required configuration parameter in "Application" configuration category. <br>
 *
 */
#define IoTPConfig_Application_appId    "Application.appId"

/**
 *  Application.authToken is a required configuration parameter in "Application" configuration category. <br>
 *
 */
#define IoTPConfig_Application_authToken    "Application.authToken"

/**
 *  Application.APIKey is a required configuration parameter in "Application" configuration category. <br>
 *
 */
#define IoTPConfig_Application_APIKey    "Application.APIKey"

/**
 *  Debug.logLevel is an optional configuration parameter in "Debug" configuration category. <br>
 *
 */
#define IoTPConfig_Debug_logLevel    "Debug.logLevel"

/**
 *  Debug.MQTTTraceLevel is an optional configuration parameter in "Debug" configuration category. <br>
 *
 */
#define IoTPConfig_Debug_MQTTTraceLevel    "Debug.MQTTTraceLevel"

/*
 * IoTPConfig_setLogHandler: Sets a Log Handler
 *
 * @param type           - Type of handler (IoTPLogHanlderType)
 *
 * @param handler        - Pointer to Log handler
 *
 * @return IoTP_RC  - IoTP_SUCCESS for success or IoTP_RC_*
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
