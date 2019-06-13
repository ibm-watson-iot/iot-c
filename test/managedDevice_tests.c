/*******************************************************************************
 * Copyright (c) 2019 IBM Corp.
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
#include "iotp_managedDevice.h"

/*
 * validateManagedDevice_tests.c: IBM Watson IoT Platform C Client Managed Device API validation tests
 *
 * This file contains test functions to test validation of following APIs:
 * - IoTPManagedDevice_create
 * - IoTPManagedDevice_destroy
 * - IoTPManagedDevice_setMQTTLogHandler
 * - IoTPManagedDevice_connect
 * - IoTPManagedDevice_disconnect
 * - IoTPManagedDevice_sendEvent
 * - IoTPManagedDevice_setCommandsHandler
 * - IoTPManagedDevice_subscribeToCommands
 * - IoTPManagedDevice_handleCommand
 * - IoTPManagedDevice_unsubscribeFromCommands
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
int testManagedDevice_create(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedDevice *managedDevice = NULL;

    /* error tests */
    rc = IoTPManagedDevice_create(NULL, config);
    TEST_ASSERT("IoTPManagedDevice_create: Create managedDevice with NULL managedDevice handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPManagedDevice_create(&managedDevice, NULL);
    TEST_ASSERT("IoTPManagedDevice_create: Create managedDevice with NULL config", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPManagedDevice_destroy(NULL);
    TEST_ASSERT("IoTPManagedDevice_create: Destroy NULL managedDevice handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    /* success tests */
    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPManagedDevice_create: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_create(&managedDevice, config);
    TEST_ASSERT("IoTPManagedDevice_create: Create managedDevice with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_destroy(managedDevice);
    TEST_ASSERT("IoTPManagedDevice_create: Destroy a valid managedDevice handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPManagedDevice_create: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: MQTT Log handler setup */
int testManagedDevice_setMQTTLogHandler(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedDevice *managedDevice = NULL;

    /* Error tests */
    rc = IoTPManagedDevice_setMQTTLogHandler(NULL, NULL);
    TEST_ASSERT("IoTPManagedDevice_setMQTTLogHandler: Invalid log handler", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPManagedDevice_setMQTTLogHandler: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_create(&managedDevice, config);
    TEST_ASSERT("IoTPManagedDevice_setMQTTLogHandler: Create managedDevice with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_setMQTTLogHandler(managedDevice, &MQTTTraceCallback);
    TEST_ASSERT("IoTPManagedDevice_setMQTTLogHandler: Valid log handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_connect(managedDevice);
    TEST_ASSERT("IoTPManagedDevice_setMQTTLogHandler: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);
    /* TEST_ASSERT("IoTPManagedDevice_setMQTTLogHandler: Verify loghandler", mqttLogCallbackActive == 1, "rcE=%d rcA=%d", mqttLogCallbackActiveExp, mqttLogCallbackActive); */
    rc = IoTPManagedDevice_destroy(managedDevice);
    TEST_ASSERT("IoTPManagedDevice_setMQTTLogHandler: Destroy a valid managedDevice handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPManagedDevice_setMQTTLogHandler: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: Device connect/disconnect/send */
int testManagedDevice_connect(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedDevice *managedDevice = NULL;
    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPManagedDevice_connect: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    IoTPConfig_readEnvironment(config);
    rc = IoTPManagedDevice_create(&managedDevice, config);
    TEST_ASSERT("IoTPManagedDevice_connect: Create managedDevice with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_connect(NULL);
    TEST_ASSERT("IoTPManagedDevice_connect: NULL", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPManagedDevice_connect(managedDevice);
    TEST_ASSERT("IoTPManagedDevice_connect: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_disconnect(managedDevice);
    TEST_ASSERT("IoTPManagedDevice_connect: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_destroy(managedDevice);
    TEST_ASSERT("IoTPManagedDevice_connect: Destroy a valid managedDevice handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPManagedDevice_connect: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: Send event - error cases */
int testManagedDevice_sendEventVal(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedDevice *managedDevice = NULL;

    rc = IoTPManagedDevice_sendEvent(managedDevice, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Invalid managedDevice object", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_create(&managedDevice, config);
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Create managedDevice with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedDevice_sendEvent(managedDevice, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Invalid event ID", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPManagedDevice_sendEvent(managedDevice, "status", NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Invalid format", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPManagedDevice_sendEvent(managedDevice, "status", NULL, "json", -1, NULL);
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Invalid QoS=-1", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_INVALID_VALUE, rc);
    rc = IoTPManagedDevice_sendEvent(managedDevice, "status", NULL, "json", 3, NULL);
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Invalid QoS=3", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_INVALID_VALUE, rc);
    rc = IoTPManagedDevice_destroy(managedDevice);
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Destroy a valid managedDevice handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPManagedDevice_sendEventVal Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: ManagedDevice send event */
int testManagedDevice_sendEvent(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedDevice *managedDevice = NULL;
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    int i = 0;

    rc = IoTPConfig_create(&config, "./wiotpdev.yaml");
    TEST_ASSERT("testManagedDevice_sendEvent: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    /* get org id, and managedDevice token from environment */
    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("testManagedDevice_sendEvent: Read config from environment", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPManagedDevice_create(&managedDevice, config);
    TEST_ASSERT("testManagedDevice_sendEvent: Create managedDevice with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPManagedDevice_setMQTTLogHandler(managedDevice, &MQTTTraceCallback);
    TEST_ASSERT("testManagedDevice_sendEvent: Set MQTT Trace handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPManagedDevice_connect(managedDevice);
    TEST_ASSERT("testManagedDevice_sendEvent: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);

    for (i=0; i<2; i++) {
        rc = IoTPManagedDevice_sendEvent(managedDevice,"status","json", data , QoS0, NULL);
        TEST_ASSERT("testManagedDevice_sendEvent: Send event QoS0", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        sleep(5);
    }

    rc = IoTPManagedDevice_disconnect(managedDevice);
    TEST_ASSERT("testManagedDevice_sendEvent: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    sleep(2);

    rc = IoTPManagedDevice_destroy(managedDevice);
    TEST_ASSERT("testManagedDevice_sendEvent: Destroy a valid managedDevice handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("testManagedDevice_sendEvent: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


int main(void)
{
    int rc = 0;
    int (*tests[])() = {testManagedDevice_create, testManagedDevice_setMQTTLogHandler, testManagedDevice_sendEventVal, testManagedDevice_connect, testManagedDevice_sendEvent};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Managed Device API Tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
    }

    testEnd("IBM IoT Platform Client: Managed Device API Tests", count);

    return rc;
}

