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

#if !defined(IOTP_DEVICE_H_)
#define IOTP_DEVICE_H_

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


/*! \page iotpdevice IoTP C Device Client Library

   This library exposes functions and declarations to build IoTP device client.

   - Add details on how to develop device clients using APIs exposed by this linrary.

 */

/**
 * A handle representing an IBM Watson IoT Platform MQTT device client. 
 * A valid client handle is available following a successful 
 * call to IoTPDevice_create()
 */
typedef void * IoTPDevice;


/**
 * IoTPDevice_create: Creates IBM Watson IoT platform device client.
 *
 * This function creates IBM Watson IoT Platform Device client. 
 * The client handle is initialized with IoTPConfig object.
 *
 * @param device         - A pointer to IoTPDevice handle.
 * 
 * @param config         - A pointer to IoTPConfig handle.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                       - IoTP_RC_INVALID_NOMEM if system runs out of memory
 *
 * Use IoTPDevice_destroy() API, to destroy a client initialized using this API.
 *
 */
DLLExport IoTP_RC IoTPDevice_create(IoTPDevice **device, IoTPConfig *config);

/**
 * IoTPDevice_setMQTTLogHandler: Sets a callback to handle client library log and trace  messages
 *
 * @param device         - A pointer to IoTPDevice handle.
 * 
 * @param cb             - Pointer to log handler
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 */
DLLExport IoTP_RC IoTPDevice_setMQTTLogHandler(IoTPDevice *device, IoTPLogHandler *cb);


/**
 * IoTPDevice_destroy: Destroys IBM Watson IoT client.
 *
 * This function destroys IBM Watson IoT Platform Device Client, previously initialized using
 * IoTPDevice_create() API.
 * 
 * @param device         - A pointer to IoTPDevice handle.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *
 */
DLLExport IoTP_RC IoTPDevice_destroy(IoTPDevice *device);


/**
 * IoTPDevice_connect: Connects the IBM Watson IoT device client to Watson IoT Platform service.
 *
 * @param device         - A pointer to IoTPDevice handle.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                         MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *                         MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *
 */
DLLExport IoTP_RC IoTPDevice_connect(IoTPDevice *device);


/**
 * IoTPDevice_disconnect: Disconnects the IBM Watson IoT device client to Watson IoT Platform service.
 *
 * @param device         - A pointer to IoTPDevice handle.
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *                         MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *                         MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *
 */
DLLExport IoTP_RC IoTPDevice_disconnect(IoTPDevice *device);


/**
 * IoTPDevice_setCommandHandler: Sets the Command Callback function. 
 *                               To receive the commands from Watson IoT Platform, this must be set.
 *
 * @param device         - A valid device handle
 *
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE if handle is not valid
 *             
 */
DLLExport IoTP_RC IoTPDevice_setCommandHandler(IoTPDevice *device, IoTPCallbackHandler cb);


/**
 * IoTPDevice_subscribeToCommands: Subscribe to commands for the device.
 *
 * If IoTPCallbackHandler is set using IoTPDevice_setCommandHandler() API, the IoTP client 
 * will receive command specified by "commandName" parameter, sent by WIoTP service.
 *
 * @param device         - A valid device handle
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
DLLExport IoTP_RC IoTPDevice_subscribeToCommands(IoTPDevice *device, char *commandId, char *formatString);


/**
 * IoTPDevice_handleCommand: Subscribes to a specific command and sets a command handler for the command. 
 *
 * @param device         - A valid device handle
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
DLLExport IoTP_RC IoTPDevice_handleCommand(IoTPDevice *device, IoTPCallbackHandler cb, char *commandId, char *formatString);


/**
 * IoTPDevice_unsubscribeFromCommands: Unsubscribe to commands for the device.
 *
 * @param device         - A valid device handle
 *
 * @param commandId        - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 *
 * @param formatString     - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 *
 * @return IoTP_RC  - Returns one of the following codes:
 *                       - IoTP_SUCCESS for success
 *                       - IoTP_RC_INVALID_HANDLE - if handle in invalid
 *                       - IoTP_RC_PARAM_NULL_VALUE - if parameter value is NULL or empty string.
 *                       - MQTTCLIENT_* - if IBM IoT Client is linked with Paho MQTT Synchronus client library
 *                       - MQTTASYNC_*  - if IBM IoT Client is linked with Paho MQTT Asynchronus client library
 *
 */
DLLExport IoTP_RC IoTPDevice_unsubscribeFromCommands(IoTPDevice *device, char *commandId, char *formatString);


/**
 * IoTPDevice_sendEvent: Publishs events from the device to the IBM Watson IoT service
 *
 * @param device         - A valid device handle
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
DLLExport IoTP_RC IoTPDevice_sendEvent(IoTPDevice *device, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);


#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_DEVICE_H_ */

