/*******************************************************************************
 * Copyright (c) 2017-2018 IBM Corp.
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
 *
 * Contrinutors:
 *    Ranjan Dasgupta         - Initial drop
 *
 *******************************************************************************/

#include "iotp_application.h"
#include "iotp_internal.h"

/* Application APIs are essentially wrapper functions of IoTPClient APIs */

/* Creates a application handle */
IoTP_RC IoTPApplication_create(IoTPApplication **application, IoTPConfig *config) 
{
    IoTP_RC rc = IoTP_SUCCESS;

    rc = iotp_client_create((void **)application, config, IoTPClient_gateway);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPGateway: rc=%d", rc);
    }

    return rc;
}

/* Set MQTT Trace handler */
IoTP_RC IoTPApplication_setMQTTLogHandler(IoTPApplication *application, IoTPLogHandler *cb)
{
    IoTP_RC rc = IoTP_SUCCESS;
    rc = iotp_client_setMQTTLogHandler((void *)application, cb);
    return rc;
}

/* Disconnect and destroys a application handle */
IoTP_RC IoTPApplication_destroy(IoTPApplication *application)
{
    IoTP_RC rc = IoTP_SUCCESS;

    rc = iotp_client_destroy((void *)application);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to destroy IoTPGateway: rc=%d", rc);
    }

    return rc;
}

/* Connects to WIoTP */
IoTP_RC IoTPApplication_connect(IoTPApplication *application)
{
    IoTP_RC rc = IoTP_SUCCESS;

    rc = iotp_client_connect((void *)application);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Application failed to connect");
    }

    return rc;
}

/* Disconnect client from WIoTP */
IoTP_RC IoTPApplication_disconnect(IoTPApplication *application)
{
    IoTP_RC rc = IoTP_SUCCESS;

    rc = iotp_client_disconnect((void *)application);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Application failed to disconnect");
    }

    return rc;
}

/* Sends Event on behalf of a device */
IoTP_RC IoTPApplication_sendEvent(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Sanity check */
    if ( !typeId || !deviceId || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IoTP_RC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Send event. topic: %s", topic);

    rc = iotp_client_publish((void *)application, topic, data, qos);

    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to send event: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Sends command to a device */
IoTP_RC IoTPApplication_sendCommand(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Sanity check */
    if ( !typeId || !deviceId || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IoTP_RC_PARAM_NULL_VALUE;
        LOG(WARN, "Received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Send command. topic: %s", topic);

    rc = iotp_client_publish((void *)application, topic, data, qos);

    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to send command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Set Event Handler */
IoTP_RC IoTPApplication_setEventHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId, char *eventId, char *formatString)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Set event handler. topic: %s", topic);

    rc = iotp_client_setHandler((void *)application, topic, IoTP_Handler_Event, cb);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to set global command handler");
    }

    return rc;
}

/* Subscribe to events */
IoTP_RC IoTPApplication_subscribeToEvents(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *formatString)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Subscribe to event. topic: %s", topic);

    rc = iotp_client_subscribe((void *)application, topic, QoS0);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the event: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Unsubscribe from events */
IoTP_RC IoTPApplication_unsubscribeFromEvents(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *formatString)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Unsubscribe from event. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)application, topic);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the event: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Set Command Handler */
IoTP_RC IoTPApplication_setCommandHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Set command handler. topic: %s", topic);

    rc = iotp_client_setHandler((void *)application, topic, IoTP_Handler_Command, cb);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to set command handler");
    }

    return rc;
}

/* Subscribe to commands */
IoTP_RC IoTPApplication_subscribeToCommands(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Subscribe to command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)application, topic, QoS0);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to the command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Unsubscribe from commands */
IoTP_RC IoTPApplication_unsubscribeFromCommands(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Unsubscribe from command. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)application, topic);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe from the command: %s rc=%d", topic, rc);
    }

    return rc;
}


/* Set Device Monitoring message Handler */
IoTP_RC IoTPApplication_setDeviceMonitoringHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/mon";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) - 3;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId);

    LOG(DEBUG,"Set device monitoring message handler. topic: %s", topic);

    rc = iotp_client_setHandler((void *)application, topic, IoTP_Handler_DeviceMonitoring, cb);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to set device monitoring message handler");
    }

    return rc;
}

/* Subscribe to device monitoring message */
IoTP_RC IoTPApplication_subscribeToDeviceMonitoringMessages(IoTPApplication *application, char *typeId, char *deviceId)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/mon";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) - 3;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId);

    LOG(DEBUG,"Subscribe to device monitoring message. topic: %s", topic);

    rc = iotp_client_subscribe((void *)application, topic, QoS0);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to device monitoring messages: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Unsubscribe from device monitoring message */
IoTP_RC IoTPApplication_unsubscribeFromDeviceMonitoringMessages(IoTPApplication *application, char *typeId, char *deviceId)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/mon";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) - 3;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId);

    LOG(DEBUG,"Unsubscribe to device monitoring message. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)application, topic);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe from device monitoring messages: %s rc=%d", topic, rc);
    }

    return rc;
}


/* Set Applocation Monitoring message Handler */
IoTP_RC IoTPApplication_setAppMonitoringHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *appId)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/app/%s/mon";
    int len = strlen(format) + strlen(appId) - 1;
    char topic[len];
    snprintf(topic, len, format, appId);

    LOG(DEBUG,"Set application monitoring message handler. topic: %s", topic);

    rc = iotp_client_setHandler((void *)application, topic, IoTP_Handler_AppMonitoring, cb);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to set application monitoring message handler");
    }

    return rc;
}

/* Subscribe to application monitoring message */
IoTP_RC IoTPApplication_subscribeToAppMonitoringMessages(IoTPApplication *application, char *appId)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/app/%s/mon";
    int len = strlen(format) + strlen(appId) - 1;
    char topic[len];
    snprintf(topic, len, format, appId);

    LOG(DEBUG,"Subscribe to application monitoring message. topic: %s", topic);

    rc = iotp_client_subscribe((void *)application, topic, QoS0);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe to application monitoring messages: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Unsubscribe from application monitoring message */
IoTP_RC IoTPApplication_unsubscribeFromAppMonitoringMessages(IoTPApplication *application, char *appId)
{
    IoTP_RC rc = IoTP_SUCCESS;

    /* Set topic string */
    char *format = "iot-2/app/%s/mon";
    int len = strlen(format) + strlen(appId) - 1;
    char topic[len];
    snprintf(topic, len, format, appId);

    LOG(DEBUG,"Unsubscribe from application monitoring message. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)application, topic);
    if ( rc != IoTP_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe from application monitoring messages: %s rc=%d", topic, rc);
    }

    return rc;
}


