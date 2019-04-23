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
 * A handle representing gateway client of IBM Watson IoT Platform service.
 * A valid client handle is available following a successful 
 * call to IoTPGateway_create() API.
 */
typedef void * IoTPGateway;

/**
 * The IoTPGateway_create() API creates a gateway client that can connect to 
 * IBM Watson IoT Platform service. The gateway client is created using configuration
 * specified by IoTP Config object.
 *
 * @param gateway        - A pointer in which to return the newly created IoTPGateway handle.
 * @param config         - A pointer to IoTP config handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 * @remark Use IoTPGateway_destroy() API, to destroy a gateway client created using this API.
 */
DLLExport IOTPRC IoTPGateway_create(IoTPGateway **gateway, IoTPConfig *config);

/**
 * The IoTPGateway_setMQTTLogHandler() API sets a callback to handle Paho MQTT client 
 * library log and trace  messages.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - Pointer to log handler
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_setMQTTLogHandler(IoTPGateway *gateway, IoTPLogHandler *cb);

/**
 * The IoTPGateway_destroy() API destroys the gateway client handle
 * previously created by using IoTPGateway_create() API.
 * 
 * @param gateway        - A pointer to IoTP gateway handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_destroy(IoTPGateway *gateway);

/**
 * The IoTPGateway_connect() API connects the gateway client to IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_connect(IoTPGateway *gateway);

/**
 * The IoTPGateway_disconnect() API disconnects the gateway client from IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_disconnect(IoTPGateway *gateway);

/**
 * The IoTPGateway_setCommandHandler() API sets the Command Callback function,
 * to receive the commands from IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_setCommandHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);

/**
 * The IoTPGateway_subscribeToCommands() API subscribes to commands for the gateway.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPGateway_setCommandHandler() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_subscribeToCommands(IoTPGateway *gateway, char *commandId, char *formatString);

/**
 * The IoTPGateway_handleCommand() API subscribes to a specific command and 
 * sets a command handler for the command. 
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_handleCommand(IoTPGateway *gateway, IoTPCallbackHandler cb, char *commandId, char *formatString);

/**
 * The IoTPGateway_unsubscribeFromCommands() API unsubscribes from commands for the gateway.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_unsubscribeFromCommands(IoTPGateway *gateway, char *commandId, char *formatString);

/**
 * The IoTPGateway_subscribeToDeviceCommands() API subscribes commands on behalf of devices 
 * registered with the IBM Watson IoT Platform service via this gateway.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPGateway_setCommandHandler() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_subscribeToDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString);

/**
 * The IoTPGateway_unsubscribeFromDeviceCommands() API unsubscribes from commands 
 * on behalf of devices registered with the IBM Watson IoT Platform service via this gateway.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_unsubscribeFromDeviceCommands(IoTPGateway *gateway, char *typeId, char *deviceId, char *commandId, char *formatString);

/**
 * The IoTPGateway_sendEvent() API sends events from the gateway to the IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC       - Returns IOTPRC_SUCCESS on success or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_sendEvent(IoTPGateway *gateway, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

/**
 * The IoTPGateway_sendDeviceEvent() API sends events on behalf of a deviec to the IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_sendDeviceEvent(IoTPGateway *gateway, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

/**
 * The IoTPGateway_setNotificationHandler() API sets a notification callback function, 
 * to receive the notifications from IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_setNotificationHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);

/**
 * The IoTPGateway_subscribeToNotifications() subscribes to gateway notifications.
 * To receive the notification, a notification callback function must be registered
 * using IoTPGateway_setNotificationHandler() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_subscribeToNotifications(IoTPGateway *gateway, char *typeId, char *deviceId);

/**
 * The IoTPGateway_unsubscribeFromNotifications() API unsubscribes from gateway notifications,
 * previously subscribed to, using IoTPGateway_subscribeToNotifications() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_unsubscribeFromNotifications(IoTPGateway *gateway, char *typeId, char *deviceId);

/**
 * The IoTPGateway_setMonitoringMessageHandler() API sets a monitoring messages callback function, 
 * to receive the monitoring messages from IBM Watson IoT Platform service.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_setMonitoringMessageHandler(IoTPGateway *gateway, IoTPCallbackHandler cb);

/**
 * The IoTPGateway_subscribeToMonitoringMessages() API subscribes to monitoring messages.
 * To receive the monitoring messages, a monitoring message callback function must be registered
 * using IoTPGateway_setMonitoringMessageHandler() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_subscribeToMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId);

/**
 * The IoTPGateway_unsubscribeFromMonitoringMessages() API unsubscribes from monitoring messages,
 * previously subscribed to, using IoTPGateway_subscribeToMonitoringMessages() API.
 *
 * @param gateway        - A pointer to IoTP gateway handle.
 * @param typeId         - Device type ID
 * @param deviceId       - Device ID
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPGateway_unsubscribeFromMonitoringMessages(IoTPGateway *gateway, char *typeId, char *deviceId);


#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_GATEWAY_H_ */

