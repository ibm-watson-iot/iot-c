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

#if !defined(IOTP_GATEWAY_H_)
#define IOTP_GATEWAY_H_

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


/*! \page iotpgateway IoTP C Gateway Client Library

   This library exposes functions and declarations to build IoTP gateway client.

   - Add details on how to develop gateway clients using APIs exposed by this linrary.

 */


/**
 * A handle representing an IBM Watson IoT Platform MQTT gateway client. 
 * A valid client handle is available following a successful 
 * call to IoTPGateway_create()
 */
typedef void * IoTPGateway;


/**
 * IoTPGateway_create: Creates IBM Watson IoT platform gateway client.
 *
 * This function creates IBM Watson IoT Platform Gateway client. 
 * The client handle is initialized with IoTPConfig object.
 *
 * @param gateway        - A pointer to IoTPGateway handle.
 * 
 * @param config         - A pointer to IoTPConfig handle.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                       - IoTP_RC_INVALID_NOMEM if system runs out of memory
 *
 * Use IoTPGateway_destroy() API, to destroy a client initialized using this API.
 *
 */
DLLExport IoTP_RC IoTPGateway_create(IoTPGateway **gateway, IoTPConfig *config);


/**
 * IoTPGateway_setMQTTLogHandler: Sets a callback to handle client library log and trace  messages
 *
 * @param gateway        - A pointer to IoTPGateway handle.
 *
 * @param cb             - Pointer to log handler
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 */
DLLExport IoTP_RC IoTPGateway_setMQTTLogHandler(IoTPGateway *gateway, IoTPLogHandler *cb);


/**
 * IoTPGateway_setLogHandler: Sets a callback to handle client library log and trace  messages
 *
 * @param gateway        - A pointer to IoTPGateway handle.
 *
 * @param cb             - Pointer to log handler
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 */
DLLExport IoTP_RC IoTPGateway_setLogHandler(IoTPGateway *gateway, IoTPLogHandler cb);


/**
 * IoTPGateway_destroy: Destroys IBM Watson IoT gateway client.
 *
 * This function destroys IBM Watson IoT Platform Gateway Client, previously initialized using
 * IoTPGateway_create() API.
 * 
 * @param gateway        - A pointer to IoTPGateway handle.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *
 */
DLLExport IoTP_RC IoTPGateway_destroy(IoTPGateway *gateway);


/**
 * IoTPGateway_connect: Connects the IBM Watson IoT gateway client to Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTPGateway handle.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                         MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *                         MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_connect(IoTPGateway *gateway);


/**
 * IoTPGateway_disconnect: Disconnects the IBM Watson IoT gateway client to Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTPGateway handle.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                         MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *                         MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_disconnect(IoTPGateway *gateway);


/**
 * IoTPGateway_setCommandHandler: Sets the Command Callback function. 
 *                               To receive the commands from Watson IoT Platform, this must be set.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *             
 */
DLLExport IoTP_RC IoTPGateway_setCommandHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);


/**
 * IoTPGateway_subscribeToCommands: Subscribe to commands for the gateway.
 *
 * If IoTPCallbackHandler is set using IoTPGateway_setCommandHandler() API, the IoTP client 
 * will receive command specified by "commandId" parameter, sent by WIoTP service.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_subscribeToCommands(IoTPGateway *gateway, char *commandId, char *formatString);


/**
 * IoTPGateway_handleCommand: Subscribes to a specific command and sets a command handler for the command. 
 *
 * @param gateway        - A valid gateway handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 *
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *             
 */
DLLExport IoTP_RC IoTPGateway_handleCommand(IoTPGateway *gateway, IoTPCallbackHandler cb, char *commandId, char *formatString);


/**
 * IoTPGateway_unsubscribeFromCommands: Unsubscribe to commands for the gateway.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_unsubscribeFromCommands(IoTPGateway *gateway, char *commandId, char *formatString);


/**
 * IoTPGateway_subscribeFromDeviceCommands: Subscribes commands on behalf of devices registered with 
 *                                          the IBM Watson IoT service via this for the gateway.
 *
 * If IoTPCallbackHandler is set using IoTPGateway_setCommandHandler() API, the IoTP client
 * will receive command specified by "commandId" parameter, sent by WIoTP service.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_subscribeToDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString);


/**
 * IoTPGateway_unsubscribeFromDeviceCommands: Unsubscribes from commands on behalf of devices registered with 
 *                                            the IBM Watson IoT service via this for the gateway.
 *
 * If IoTPCallbackHandler is set using IoTPGateway_setCommandHandler() API, the IoTP client
 * will receive command specified by "commandId" parameter, sent by WIoTP service.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 *
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_unsubscribeFromDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString);


/**
 * IoTPGateway_sendEvent: Sends events from the gateway to the IBM Watson IoT service
 *
 * @param gateway        - A valid gateway handle
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
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *
 */
DLLExport IoTP_RC IoTPGateway_sendEvent(IoTPGateway *gateway, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);


/**
 * IoTPGateway_sendDeviceEvent: Sends events on behalf of a deviec to the IBM Watson IoT service
 *
 * @param gateway        - A valid gateway handle
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
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *
 */
DLLExport IoTP_RC IoTPGateway_sendDeviceEvent(IoTPGateway *gateway, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);


/**
 * IoTPGateway_setNotificationHandler: Sets a notification callback function, 
 *                                     to receive the notifications from Watson IoT Platform.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *
 */
DLLExport IoTP_RC IoTPGateway_setNotificationHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);


/**
 * IoTPGateway_subscribeToNotifications: Subscribes to gateway notifications. 
 *
 * @param gateway        - A valid gateway handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_subscribeToNotifications(IoTPGateway *gateway, char *typeId, char *deviceId);


/**
 * IoTPGateway_unsubscribeFromNotifications: Unsubscribes from gateway notifications. 
 *
 * @param gateway        - A valid gateway handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_unsubscribeFromNotifications(IoTPGateway *gateway, char *typeId, char *deviceId);


/**
 * IoTPGateway_setMonitoringMessageHandler: Sets a monitoring messages callback function, 
 *                                          to receive the monitoring messages from Watson IoT Platform.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *
 */
DLLExport IoTP_RC IoTPGateway_setMonitoringMessageHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);


/**
 * IoTPGateway_subscribeToMonitoringMessages: Subscribes to monitoring messages.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_subscribeToMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId);


/**
 * IoTPGateway_unsubscribeFromMonitoringMessages: Unsubscribes to monitoring messages.
 *
 * @param gateway        - A valid gateway handle
 *
 * @param typeId         - Device type ID
 *
 * @param deviceId       - Device ID
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *
 */
DLLExport IoTP_RC IoTPGateway_unsubscribeFromMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId);


#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_GATEWAY_H_ */

