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
 *
 * Contrinutors:
 *    Ranjan Dasgupta         - Initial drop
 *
 *******************************************************************************/

#include "iotp_application.h"
#include "iotp_internal.h"


/* Creates an application handle */
IOTPRC IoTPApplication_create(IoTPApplication **application, IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_create((void **)application, config, IoTPClient_application);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create an application handle. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sets MQTT trace handler */
IOTPRC IoTPApplication_setMQTTLogHandler(IoTPApplication *application, IoTPLogHandler *cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setMQTTLogHandler((void *)application, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(WARN, "Failed to set MQTT log handler. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Destroys an application handle */
IOTPRC IoTPApplication_destroy(IoTPApplication *application)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_destroy((void *)application);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to destroy an application handle. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Connects to WIoTP */
IOTPRC IoTPApplication_connect(IoTPApplication *application)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_connect((void *)application);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to connect. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Disconnects from WIoTP */
IOTPRC IoTPApplication_disconnect(IoTPApplication *application)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_disconnect((void *)application);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Faiiled to disconnect, rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sends Event on behalf of a device or gateway */
IOTPRC IoTPApplication_sendEvent(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
    if ( qos != QoS0 && qos != QoS1 && qos != QoS2 ) {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        LOG(WARN, "Invalid QoS. qos: %d | rc: %d | reason: %s", qos, rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Send event. topic: %s", topic);

    rc = iotp_client_publish((void *)application, topic, data, qos, props);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send event. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sends command to a device */
IOTPRC IoTPApplication_sendCommand(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
    if ( qos != QoS0 && qos != QoS1 && qos != QoS2 ) {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        LOG(WARN, "Invalid QoS. qos: %d | rc: %d | reason: %s", qos, rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Send command. topic: %s", topic);

    rc = iotp_client_publish((void *)application, topic, data, qos, props);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sets event handler */
IOTPRC IoTPApplication_setEventHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId, char *eventId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !cb || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Set event handler. topic: %s", topic);

    rc = iotp_client_setHandler((void *)application, topic, IoTP_Handler_AppEvent, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set handler. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Subscribe to events */
IOTPRC IoTPApplication_subscribeToEvents(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Subscribe to event. topic: %s", topic);

    rc = iotp_client_subscribe((void *)application, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Unsubscribe from events */
IOTPRC IoTPApplication_unsubscribeFromEvents(IoTPApplication *application, char *typeId, char *deviceId, char *eventId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(eventId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, eventId, formatString);

    LOG(DEBUG,"Unsubscribe from event. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)application, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Set Command Handler */
IOTPRC IoTPApplication_setCommandHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !cb || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Set command handler. topic: %s", topic);

    rc = iotp_client_setHandler((void *)application, topic, IoTP_Handler_Command, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set handler. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Subscribe to commands */
IOTPRC IoTPApplication_subscribeToCommands(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Subscribe to command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)application, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Unsubscribe from commands */
IOTPRC IoTPApplication_unsubscribeFromCommands(IoTPApplication *application, char *typeId, char *deviceId, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) + strlen(commandId) + strlen(formatString) - 7;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId, commandId, formatString);

    LOG(DEBUG,"Unsubscribe from command. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)application, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Set Device Monitoring message Handler */
IOTPRC IoTPApplication_setDeviceMonitoringHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !cb || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/mon";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) - 3;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId);

    LOG(DEBUG,"Set device monitoring message handler. topic: %s", topic);

    rc = iotp_client_setHandler((void *)application, topic, IoTP_Handler_DeviceMonitoring, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set handler. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Subscribe to device monitoring message */
IOTPRC IoTPApplication_subscribeToDeviceMonitoringMessages(IoTPApplication *application, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;
 
    /* Sanity check */
    if ( !application || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/mon";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) - 3;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId);

    LOG(DEBUG,"Subscribe to device monitoring message. topic: %s", topic);

    rc = iotp_client_subscribe((void *)application, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Unsubscribe from device monitoring message */
IOTPRC IoTPApplication_unsubscribeFromDeviceMonitoringMessages(IoTPApplication *application, char *typeId, char *deviceId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !typeId || *typeId == '\0' || !deviceId || *deviceId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/type/%s/id/%s/mon";
    int len = strlen(format) + strlen(typeId) + strlen(deviceId) - 3;
    char topic[len];
    snprintf(topic, len, format, typeId, deviceId);

    LOG(DEBUG,"Unsubscribe to device monitoring message. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)application, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Set Applocation Monitoring message Handler */
IOTPRC IoTPApplication_setAppMonitoringHandler(IoTPApplication *application, IoTPCallbackHandler cb, char *appId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !cb || !appId || *appId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/app/%s/mon";
    int len = strlen(format) + strlen(appId) - 1;
    char topic[len];
    snprintf(topic, len, format, appId);

    LOG(DEBUG,"Set application monitoring message handler. topic: %s", topic);

    rc = iotp_client_setHandler((void *)application, topic, IoTP_Handler_AppMonitoring, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set handler. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Subscribe to application monitoring message */
IOTPRC IoTPApplication_subscribeToAppMonitoringMessages(IoTPApplication *application, char *appId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !appId || *appId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/app/%s/mon";
    int len = strlen(format) + strlen(appId) - 1;
    char topic[len];
    snprintf(topic, len, format, appId);

    LOG(DEBUG,"Subscribe to application monitoring message. topic: %s", topic);

    rc = iotp_client_subscribe((void *)application, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Unsubscribe from application monitoring message */
IOTPRC IoTPApplication_unsubscribeFromAppMonitoringMessages(IoTPApplication *application, char *appId)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !application || !appId || *appId == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | Reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
 
    /* Set topic string */
    char *format = "iot-2/app/%s/mon";
    int len = strlen(format) + strlen(appId) - 1;
    char topic[len];
    snprintf(topic, len, format, appId);

    LOG(DEBUG,"Unsubscribe from application monitoring message. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)application, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | Reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}


