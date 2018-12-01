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
 * - IoTPDevice_setCommandHandler
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
    int rc = IoTP_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;

    /* error tests */
    rc = IoTPDevice_create(NULL, config);
    TEST_ASSERT("IoTPDevice_create: Create device with NULL device handle", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);
    rc = IoTPDevice_create(&device, NULL);
    TEST_ASSERT("IoTPDevice_create: Create device with NULL config", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);
    rc = IoTPDevice_destroy(NULL);
    TEST_ASSERT("IoTPDevice_create: Destroy NULL device handle", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);

    /* success tests */
    rc = IoTPConfig_create(&config, "./iotpclient.cfg");
    TEST_ASSERT("IoTPDevice_create: Create config object", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("IoTPDevice_create: Create device with valid config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("IoTPDevice_create: Destroy a valid device handle", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPDevice_create: Clear Config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    return rc;
}

int testDevice_setMQTTLogHandler(void)
{
    int rc = IoTP_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;

    /* Error tests */
    rc = IoTPDevice_setMQTTLogHandler(NULL, NULL);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Invalid log handler", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);

    rc = IoTPConfig_create(&config, "./iotpclient.cfg");
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Create config object", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Create device with valid config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_setMQTTLogHandler(device, &MQTTTraceCallback);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Valid log handler", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_connect(device);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Connect client", rc == -8, "rcE=%d rcA=%d", -8, rc);
    sleep(2);
    /* TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Verify loghandler", mqttLogCallbackActive == 1, "rcE=%d rcA=%d", mqttLogCallbackActiveExp, mqttLogCallbackActive); */
    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Destroy a valid device handle", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPDevice_setMQTTLogHandler: Clear Config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    return rc;
}


int testDevice_connect(void)
{
    int rc = IoTP_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;
    rc = IoTPConfig_create(&config, "./iotpclient.cfg");
    TEST_ASSERT("IoTPDevice_connect: Create config object", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("IoTPDevice_connect: Create device with valid config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_connect(NULL);
    TEST_ASSERT("IoTPDevice_connect: NULL", rc == IoTP_RC_INVALID_HANDLE, "rcE=%d rcA=%d", IoTP_RC_INVALID_HANDLE, rc);
    rc = IoTPDevice_connect(device);
    TEST_ASSERT("IoTPDevice_connect: Connect client", rc == -8, "rcE=%d rcA=%d", -8, rc);
    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("IoTPDevice_connect: Destroy a valid device handle", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPDevice_connect: Clear Config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    return rc;
}

int testDevice_disconnect(void)
{
    int rc = IoTP_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;
    rc = IoTPConfig_create(&config, "./iotpclient.cfg");
    TEST_ASSERT("IoTPDevice_disconnect: Create config object", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("IoTPDevice_disconnect: Create device with valid config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_connect(device);
    TEST_ASSERT("IoTPDevice_disconnect: Connect client", rc == -8, "rcE=%d rcA=%d", -8, rc);
    rc = IoTPDevice_disconnect(device);
    TEST_ASSERT("IoTPDevice_disconnect: Disconnect client", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("IoTPDevice_disconnect: Destroy a valid device handle", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPDevice_disconnect: Clear Config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    return rc;
}


int main(int argc, char** argv)
{
    int rc = 0;
    int (*tests[])() = {testDevice_create, testDevice_setMQTTLogHandler, testDevice_connect, testDevice_disconnect};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Device API Tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
        sleep(2);
    }

    testEnd("IBM IoT Platform Client: Device API Tests", count);

    return rc;
}

