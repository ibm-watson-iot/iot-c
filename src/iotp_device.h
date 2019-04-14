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

   - Add details on how to develop device clients using APIs exposed by this library.

 */

/**
 * A handle representing a device client of IBM Watson IoT Platform service.
 * A valid client handle is available following a successful 
 * call to IoTPDevice_create() API.
 */
typedef void * IoTPDevice;

/**
 * The IoTPDevice_create() API creates a device client that can connect to
 * IBM Watson IoT Platform service. The device client is created using configuration
 * specified by IoTP Config object.
 *
 * @param device         - A pointer in which to return the newly created IoTPDevice handle.
 * @param config         - A pointer to IoTP config handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 * @remark Use IoTPDevice_destroy() API, to destroy a client created using this API.
 */
DLLExport IOTPRC IoTPDevice_create(IoTPDevice **device, IoTPConfig *config);

/**
 * The IoTPDevice_setMQTTLogHandler() API sets a callback to handle Paho MQTT client 
 * library log and trace  messages.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param cb             - Pointer to log handler
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_setMQTTLogHandler(IoTPDevice *device, IoTPLogHandler *cb);

/**
 * The IoTPDevice_destroy() API destroys the device client handle,
 * previously created by using IoTPDevice_create() API. 
 *
 * @param device         - A pointer to IoTP device handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_destroy(IoTPDevice *device);

/**
 * The IoTPDevice_connect() API connects a device client to IBM Watson IoT Platform service.
 *
 * @param device         - A pointer to IoTP device handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_connect(IoTPDevice *device);

/**
 * The IoTPDevice_disconnect() API disconnects the device client from IBM Watson IoT Platform service.
 *
 * @param device         - A pointer to IoTP device handle.
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_disconnect(IoTPDevice *device);

/**
 * The IoTPDevice_setCommandsHandler() API sets the Command Callback function,
 * to receive the commands from IBM Watson IoT Platform servce.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_setCommandsHandler(IoTPDevice *device, IoTPCallbackHandler cb);

/**
 * The IoTPDevice_subscribeToCommands() API subscribe to commands for the device.
 * To receive command specified by "commandId" paramete, a command callback handler
 * must be registered using IoTPDevice_setCommandHandler() API.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 */
DLLExport IOTPRC IoTPDevice_subscribeToCommands(IoTPDevice *device, char *commandId, char *formatString);

/**
 * The IoTPDevice_setCommandHandler() API subscribes to a specific command and 
 * sets a command handler for the command. 
 *
 * @param device         - A pointer to IoTP device handle.
 * @param cb             - A Function pointer to the IoTPCallbackHandler. 
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_setCommandHandler(IoTPDevice *device, IoTPCallbackHandler cb, char *commandId, char *formatString);

/**
 * The IoTPDevice_unsubscribeFromCommands() API unsubscribes from commands for the device.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param commandId      - ID of command. The command ID can be any string that is valid in
 *                         MQTT protocol. Also accepts MQTT wild card character "+".
 * @param formatString   - Reponse format. e.g. json, xml, txt, csv. Also accepts MQTT wild card character "+"
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_unsubscribeFromCommands(IoTPDevice *device, char *commandId, char *formatString);

/**
 * The IoTPDevice_sendEvent() API sends events from the device to the IBM Watson IoT service.
 *
 * @param device         - A pointer to IoTP device handle.
 * @param eventId        - Event id to be published e.g status, gps
 * @param data           - Payload of the event
 * @param formatString   - Format of the event e.g json
 * @param qos            - QoS for the publish event. Supported values : QoS0, QoS1, QoS2
 * @param props          - MQTT V5 properties
 * @return IOTPRC       - Returns IOTPRC_SUCCESS onsuccess or IOTPRC_* on error
 */
DLLExport IOTPRC IoTPDevice_sendEvent(IoTPDevice *device, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props);

#if defined(__cplusplus)
 }
#endif

#endif /* IOTP_DEVICE_H_ */

