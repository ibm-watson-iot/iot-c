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

#if !defined(IOTP_APPLICATION_H_)
#define IOTP_APPLICATION_H_

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

#include <MQTTProperties.h>

#include "iotp_rc.h"
#include "iotp_utils.h"
#include "iotp_config.h"

/*
/// @endcond
*/


/*! \page iotpapp IoTP C Application Client Library

   This library exposes functions and declarations to build IoTP application client.

   - Add details on how to develop application clients using APIs exposed by this linrary.

 */


/**
 * A handle representing an IBM Watson IoT Platform MQTT application client. 
 * A valid client handle is available following a successful 
 * call to IoTPApplication_create()
 */
typedef void * IoTPApplication;


/**
 * IoTPApplication_create: Creates IBM Watson IoT platform application client.
 *
 * This function creates IBM Watson IoT Platform Application client. 
 * The client handle is initialized with IoTPConfig object.
 *
 * @param application    - A pointer to IoTPApplication handle.
 * 
 * @param config         - A pointer to IoTPConfig handle.
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 *                       - IOTPRC_INVALID_NOMEM if system runs out of memory
 *
 * Use IoTPApplication_destroy() API, to destroy a client initialized using this API.
 *
 */
DLLExport IOTPRC IoTPApplication_create(IoTPApplication **application, IoTPConfig *config);


/**
 * IoTPApplication_setMQTTLogHandler: Sets a callback to handle client library log and trace  messages
 *
 * @param application    - A pointer to IoTPApplication handle.
 *
 * @param cb             - Pointer to log handler
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 */
DLLExport IOTPRC IoTPApplication_setMQTTLogHandler(IoTPApplication *application, IoTPLogHandler *cb);


/**
 * IoTPApplication_setLogHandler: Sets a callback to handle client library log and trace messages
 *
 * @param application    - A pointer to IoTPApplication handle.
 *
 * @param cb             - Pointer to log handler
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 */
DLLExport IOTPRC IoTPApplication_setLogHandler(IoTPApplication *application, IoTPLogHandler cb);


/**
 * IoTPApplication_destroy: Destroys IBM Watson IoT client.
 *
 * This function destroys IBM Watson IoT Platform Application Client, previously initialized using
 * IoTPApplication_create() API.
 * 
 * @param application    - A pointer to IoTPApplication handle.
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 *
 */
DLLExport IOTPRC IoTPApplication_destroy(IoTPApplication *application);


/**
 * IoTPApplication_connect: Connects the IBM Watson IoT application client to Watson IoT Platform service.
 *
 * @param application    - A pointer to IoTPApplication handle.
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 *                         MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *                         MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_connect(IoTPApplication *application);


/**
 * IoTPApplication_disconnect: Disconnects the IBM Watson IoT application client to Watson IoT Platform service.
 *
 * @param application    - A pointer to IoTPApplication handle.
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 *                         MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *                         MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_disconnect(IoTPApplication *application);


/**
 * IoTPApplication_sendEvent: Publishs events for the application to the IBM Watson IoT service, or
 *                            on behalf of other devices.
 *
 * @param application    - A valid application handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param eventId        - Event id to be published e.g status, gps
 *
 * @param data           - Payload of the event
 *
 * @param formatString   - Format of the event e.g json
 *
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 *
 * @param props          - MQTT V5 properties
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *
 */
DLLExport IOTPRC IoTPApplication_sendEvent(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);


/**
 * IoTPApplication_sendCommand: Publishs a command from the application to the IBM Watson IoT service
 *
 * @param application    - A valid application handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param commandId      - Command id to be published e.g status, gps
 *
 * @param data           - Payload of the event
 *
 * @param formatString   - Format of the event e.g json
 *
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 *
 * @param props          - MQTT V5 properties
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *
 */
DLLExport IOTPRC IoTPApplication_sendCommand(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *data, char *formatString, QoS qos, MQTTProperties *props);


/**
 * IoTPApplication_setEventHandler: Sets the Event Callback function. 
 *
 * @param application    - A valid application handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param eventId        - ID of event.
 *
 * @param formatString   - Format of the event e.g json
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 *             
 */
DLLExport IOTPRC IoTPApplication_setEventHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId, char *eventId, char *formatString);


/**
 * IoTPApplication_subscribeToEvents: Subscribe to events
 *
 * @param application    - A valid application handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param eventId        - ID of event.
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *                       - IOTPRC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_subscribeToEvents(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *formatString);


/**
 * IoTPApplication_unsubscribeFromEvents: Unsubscribe from events
 *
 * @param application    - A valid application handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param eventId        - ID of event.
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *                       - IOTPRC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_unsubscribeFromEvents(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *formatString);


/**
 * IoTPApplication_setCommandHandler: Sets Command Callback function. 
 *
 * @param application    - A valid application handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param commandId      - Command ID
 *
 * @param formatString   - Format of the event e.g json
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 *             
 */
DLLExport IOTPRC IoTPApplication_setCommandHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId, char *commandId, char *formatString);


/**
 * IoTPApplication_subscribeToCommands: Subscribe to commands
 *
 * @param application    - A valid application handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param commandId      - Command ID
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *                       - IOTPRC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_subscribeToCommands(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *formatString);


/**
 * IoTPApplication_unsubscribeFromCommands: Unsubscribe from commands
 *
 * @param application    - A valid application handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param commandId      - Command ID
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *                       - IOTPRC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_unsubscribeFromCommands(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *formatString);


/**
 * IoTPApplication_setDeviceMonitoringHandler: Sets device monitoring message handler
 *
 * @param application    - A valid application handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 *             
 */
DLLExport IOTPRC IoTPApplication_setDeviceMonitoringHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId);


/**
 * IoTPApplication_subscribeToDeviceMonitoringMessages: Subscribe to device monitoring messages
 *
 * @param application    - A valid application handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *                       - IOTPRC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_subscribeToDeviceMonitoringMessages(IoTPApplication *application, char *typeId, char *deviceId);


/**
 * IoTPApplication_unsubscribeFromDeviceMonitoringMessages: Unsubscribe from device monitoring messages
 *
 * @param application    - A valid application handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *                       - IOTPRC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_unsubscribeFromDeviceMonitoringMessages(IoTPApplication *application, char *typeId, char *deviceId);



/**
 * IoTPApplication_setAppMonitoringHandler: Sets application monitoring message handler
 *
 * @param application    - A valid application handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 *
 * @param appId          - Application ID
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE if handle is not valid
 *             
 */
DLLExport IOTPRC IoTPApplication_setAppMonitoringHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *appId);


/**
 * IoTPApplication_subscribeToAppMonitoringMessages: Subscribe to application monitoring messages
 *
 * @param application    - A valid application handle
 *
 * @param appId          - Application ID
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *                       - IOTPRC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_subscribeToAppMonitoringMessages(IoTPApplication *application, char *appId);


/**
 * IoTPApplication_unsubscribeFromAppMonitoringMessages: Unsubscribe from application monitoring messages
 *
 * @param application    - A valid application handle
 *
 * @param appId          - Application ID
 *
 * @return IOTPRC  - Returns one of the following codes:
 *                       - IOTPRC_SUCCESS for success
 *                       - IOTPRC_INVALID_HANDLE - if handle in invalid
 *                       - IOTPRC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IOTPRC IoTPApplication_unsubscribeFromAppMonitoringMessages(IoTPApplication *application, char *appId);


#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_APPLICATION_H_ */

