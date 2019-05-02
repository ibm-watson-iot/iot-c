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

#include "iotp_device.h"
#include "iotp_internal.h"


/* Creates a device handle */
IOTPRC IoTPDevice_create(IoTPDevice **device, IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_create((void **)device, config, IoTPClient_device);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create device handle. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sets MQTT trace handler */
IOTPRC IoTPDevice_setMQTTLogHandler(IoTPDevice *device, IoTPLogHandler *cb) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setMQTTLogHandler((void *)device, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set MQTT Log handler. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Destroys a device handle */
IOTPRC IoTPDevice_destroy(IoTPDevice *device)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_destroy((void *)device);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to destroy device handle. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Connects to WIoTP */
IOTPRC IoTPDevice_connect(IoTPDevice *device)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_connect((void *)device);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to connect. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Disconnects from WIoTP */
IOTPRC IoTPDevice_disconnect(IoTPDevice *device)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_disconnect((void *)device);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to disconnect. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sends an event */
IOTPRC IoTPDevice_sendEvent(IoTPDevice *device, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !device || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Received NULL argument. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }
    if ( qos != QoS0 && qos != QoS1 && qos != QoS2 ) {
        rc = IOTPRC_ARGS_INVALID_VALUE;
        LOG(WARN, "Invalid QoS. qos: %d | rc: %d | reason: %s", qos, rc, IOTPRC_toString(rc));
        return rc;
    }


    /* Set topic string */
    char *format = "iot-2/evt/%s/fmt/%s";
    int len = strlen(format) + strlen(eventId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, eventId, formatString);

    LOG(DEBUG,"Send event. topic: %s", topic);

    rc = iotp_client_publish((void *)device, topic, data, qos, props);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to send. event: %s | rc: %d | reason: %s", eventId, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sets a handler for all commands */
IOTPRC IoTPDevice_setCommandsHandler(IoTPDevice *device, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)device, NULL, IoTP_Handler_Commands, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set global command handler. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}


/* Subscribes to command */
IOTPRC IoTPDevice_subscribeToCommands(IoTPDevice *device, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !device || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL arguments. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)device, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Sets a callback handler and subscribe to a command */
IOTPRC IoTPDevice_setCommandHandler(IoTPDevice *device, IoTPCallbackHandler cb, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !device || !cb || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(WARN, "Invalid or NULL argument. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* check for wild card character - not supported by this API */
    if ( strstr(commandId, "+")) {
        rc = IOTPRC_INVALID_ARGS;
        LOG(WARN, "Wild card is not supported. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* set handler */
    rc = iotp_client_setHandler((void *)device, commandId, IoTP_Handler_Command, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set command handler. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)device, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to subscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}

/* Unsubscribe from a command */
IOTPRC IoTPDevice_unsubscribeFromCommands(IoTPDevice *device, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !device || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_ARGS_NULL_VALUE;
        LOG(ERROR, "Invalid or NULL argument. rc: %d | reason: %s", rc, IOTPRC_toString(rc));
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Unsubscribe command. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)device, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to unsubscribe. topic: %s | rc: %d | reason: %s", topic, rc, IOTPRC_toString(rc));
    }

    return rc;
}


