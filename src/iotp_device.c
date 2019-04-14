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


/* Device APIs are essentially wrapper functions of internal iotp_client_*() functions */


/* Creates a WIoTP device client */
IOTPRC IoTPDevice_create(IoTPDevice **device, IoTPConfig *config) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_create((void **)device, config, IoTPClient_device);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to create IoTPDevice: rc=%d", rc);
    }

    return rc;
}

/* Set MQTT Trace handler */
IOTPRC IoTPDevice_setMQTTLogHandler(IoTPDevice *device, IoTPLogHandler *cb) 
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setMQTTLogHandler((void *)device, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set MQTT Log handler: rc=%d", rc);
    }

    return rc;
}

/* Disconnects and destroys a WIoTP device client */
IOTPRC IoTPDevice_destroy(IoTPDevice *device)
{
    IOTPRC rc = IOTPRC_SUCCESS;
    int destroyMQTTClient = 1;

    /* disconnect and destroy client */
    rc = iotp_client_destroy((void *)device, destroyMQTTClient);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to destroy IoTPDevice: rc=%d", rc);
    }

    return rc;
}

/* Connects to WIoTP */
IOTPRC IoTPDevice_connect(IoTPDevice *device)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_connect((void *)device);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to connect IoTPDevice: rc=%d", rc);
    }

    return rc;
}

/* Disconnects from WIoTP */
IOTPRC IoTPDevice_disconnect(IoTPDevice *device)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_disconnect((void *)device);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to disconnect IoTPDevice: rc=%d", rc);
    }

    return rc;
}

/* Sends event to WIoTP */
IOTPRC IoTPDevice_sendEvent(IoTPDevice *device, char *eventId, char *data, char *formatString, QoS qos, MQTTProperties *props)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !device || !eventId || *eventId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPDevice_sendEvent received NULL arguments: rc=%d", rc);
        return rc;
    }
    if ( qos != QoS0 && qos != QoS1 && qos != QoS2 ) {
        rc = IOTPRC_PARAM_INVALID_VALUE;
        LOG(WARN, "IoTPDevice_sendEvent received invalid arguments: rc=%d", rc);
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
        LOG(ERROR, "IoTPDevice failed to send event: %s rc=%d", eventId, rc);
    }

    return rc;
}

/* Sets command handler */
IOTPRC IoTPDevice_setCommandsHandler(IoTPDevice *device, IoTPCallbackHandler cb)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    rc = iotp_client_setHandler((void *)device, NULL, IoTP_Handler_Commands, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "Failed to set IoTPDevice global command handler: rc=%d", rc);
    }

    return rc;
}


IOTPRC IoTPDevice_subscribeToCommands(IoTPDevice *device, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPDevice_subscribeToCommands received invalid or NULL arguments: rc=%d", rc);
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
        LOG(ERROR, "IoTPDevice failed to subscribe to the command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Set a callback handler and subscribe to a command - not allowed is global handler is set */
IOTPRC IoTPDevice_setCommandHandler(IoTPDevice *device, IoTPCallbackHandler cb, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !cb || !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPDevice_setCommandHandler received invalid or NULL arguments: rc=%d", rc);
        return rc;
    }

    /* check for wild card character - not supported by this API */
    if ( strstr(commandId, "+")) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(WARN, "IoTPDevice_setCommandHandler does not support wild card characters: rc=%d", rc);
        return rc;
    }

    /* set handler */
    rc = iotp_client_setHandler((void *)device, commandId, IoTP_Handler_Command, cb);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPDevice failed to set command handler: rc=%d", rc);
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_subscribe((void *)device, topic, QoS0);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPDevice failed to subscribe to the command: %s rc=%d", topic, rc);
    }

    return rc;
}

/* Unsubscribe from a command */
IOTPRC IoTPDevice_unsubscribeFromCommands(IoTPDevice *device, char *commandId, char *formatString)
{
    IOTPRC rc = IOTPRC_SUCCESS;

    /* Sanity check */
    if ( !commandId || *commandId == '\0' || !formatString || *formatString == '\0' ) {
        rc = IOTPRC_PARAM_NULL_VALUE;
        LOG(ERROR, "IoTPDevice_unsubscribeFromCommands received invalid or NULL arguments, rc=%d", rc);
        return rc;
    }

    /* Set topic string */
    char *format = "iot-2/cmd/%s/fmt/%s";
    int len = strlen(format) + strlen(commandId) + strlen(formatString) - 3;
    char topic[len];
    snprintf(topic, len, format, commandId, formatString);

    LOG(DEBUG,"Subscribe command. topic: %s", topic);

    rc = iotp_client_unsubscribe((void *)device, topic);
    if ( rc != IOTPRC_SUCCESS ) {
        LOG(ERROR, "IoTPDevice failed to unsubscribe from the command: %s rc=%d", topic, rc);
    }

    return rc;
}


