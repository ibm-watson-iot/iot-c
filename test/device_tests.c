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

#include "test_utils.h"
#include "iotp_config.h"
#include "iotp_device.h"

/*
 * validateDevice_tests.c: IBM Watson IoT Platform C Client Device API validation tests
 *
 * This file contains test functions to test validation of following APIs:
 * - IoTPDevice_create
 * - IoTPDevice_destroy
 * - IoTPDevice_setMQTTLogHandler
 * - IoTPDevice_connect
 * - IoTPDevice_disconnect
 * - IoTPDevice_sendEvent
 * - IoTPDevice_setCommandsHandler
 * - IoTPDevice_subscribeToCommands
 * - IoTPDevice_handleCommand
 * - IoTPDevice_unsubscribeFromCommands
 */

int logCallbackActive = 0;
int logCallbackActiveExp = 1;
int mqttLogCallbackActive = 0;
int mqttLogCallbackActiveExp = 1;

void logCallback (int level, char * message)
{
    fprintf(stdout, "level=%d: %s\n", level, message? message:"NULL");
    fflush(stdout);
    logCallbackActive = 1;
}

void MQTTTraceCallback (int level, char * message)
{
    fprintf(stdout, "level=%d: %s\n", level, message? message:"NULL");
    fflush(stdout);
    mqttLogCallbackActive = 1;
}


/* Tests: Device object create and destroy */
int testDevice_create(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;

    /* error tests */
    rc = IoTPDevice_create(NULL, config);
    TEST_ASSERT("IoTPDevice_create: Create device with NULL device handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPDevice_create(&device, NULL);
    TEST_ASSERT("IoTPDevice_create: Create device with NULL config", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPDevice_destroy(NULL);
    TEST_ASSERT("IoTPDevice_create: Destroy NULL device handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    /* success tests */
    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPDevice_create: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("IoTPDevice_create: Create device with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("IoTPDevice_create: Destroy a valid device handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPDevice_create: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: MQTT Log handler setup */
int testDevice_setMQTTLogHandler(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;

    /* Error tests */
    rc = IoTPDevice_setMQTTLogHandler(NULL, NULL);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Invalid log handler", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Create device with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_setMQTTLogHandler(device, &MQTTTraceCallback);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Valid log handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_connect(device);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);
    /* TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Verify loghandler", mqttLogCallbackActive == 1, "rcE=%d rcA=%d", mqttLogCallbackActiveExp, mqttLogCallbackActive); */
    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Destroy a valid device handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: Device connect/disconnect/send */
int testDevice_connect(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;
    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPDevice_connect: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    IoTPConfig_readEnvironment(config);
    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("IoTPDevice_connect: Create device with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_connect(NULL);
    TEST_ASSERT("IoTPDevice_connect: NULL", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPDevice_connect(device);
    TEST_ASSERT("IoTPDevice_connect: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_disconnect(device);
    TEST_ASSERT("IoTPDevice_connect: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("IoTPDevice_connect: Destroy a valid device handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPDevice_connect: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: Send event - error cases */
int testDevice_sendEventVal(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;

    rc = IoTPDevice_sendEvent(device, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPDevice_sendEventVal: Invalid device object", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPDevice_sendEventVal: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("IoTPDevice_sendEventVal: Create device with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPDevice_sendEvent(device, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPDevice_sendEventVal: Invalid event ID", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPDevice_sendEvent(device, "status", NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPDevice_sendEventVal: Invalid format", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPDevice_sendEvent(device, "status", NULL, "json", -1, NULL);
    TEST_ASSERT("IoTPDevice_sendEventVal: Invalid QoS=-1", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_INVALID_VALUE, rc);
    rc = IoTPDevice_sendEvent(device, "status", NULL, "json", 3, NULL);
    TEST_ASSERT("IoTPDevice_sendEventVal: Invalid QoS=3", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_INVALID_VALUE, rc);
    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("IoTPDevice_sendEventVal: Destroy a valid device handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPDevice_sendEventVal: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: Device send event */
int testDevice_sendEvent(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    int i = 0;

    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("test_sendEvent: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    /* get org id, and device token from environment */
    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("test_sendEvent: Read config from environment", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("test_sendEvent: Create device with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPDevice_setMQTTLogHandler(device, &MQTTTraceCallback);
    TEST_ASSERT("test_sendEvent: Set MQTT Trace handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPDevice_connect(device);
    TEST_ASSERT("test_sendEvent: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);

    for (i=0; i<2; i++) {
        rc = IoTPDevice_sendEvent(device,"status","json", data , QoS0, NULL);
        TEST_ASSERT("test_sendEvent: Send event QoS0", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        sleep(5);
    }

    rc = IoTPDevice_disconnect(device);
    TEST_ASSERT("test_sendEvent: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    sleep(2);

    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("test_sendEvent: Destroy a valid device handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("test_sendEvent: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

int main(void)
{
    int rc = 0;
    int (*tests[])() = {testDevice_create, testDevice_setMQTTLogHandler, testDevice_sendEventVal, testDevice_connect, testDevice_sendEvent};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Device API Tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
    }

    testEnd("IBM IoT Platform Client: Device API Tests", count);

    return rc;
}

