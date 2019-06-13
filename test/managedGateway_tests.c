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
#include "iotp_managedGateway.h"

/*
 * validateManagedGateway_tests.c: IBM Watson IoT Platform C Client ManagedGateway API validation tests
 *
 * This file contains test functions to test validation of following APIs:
 * - IoTPManagedGateway_create
 * - IoTPManagedGateway_destroy
 * - IoTPManagedGateway_setMQTTLogHandler
 * - IoTPManagedGateway_connect
 * - IoTPManagedGateway_disconnect
 * - IoTPManagedGateway_sendEvent
 * - IoTPManagedGateway_setCommandsHandler
 * - IoTPManagedGateway_subscribeToCommands
 * - IoTPManagedGateway_handleCommand
 * - IoTPManagedGateway_unsubscribeFromCommands
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
int testManagedGateway_create(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedGateway *managedGateway = NULL;

    /* error tests */
    rc = IoTPManagedGateway_create(NULL, config);
    TEST_ASSERT("IoTPManagedGateway_create: Create managedGateway with NULL managedGateway handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPManagedGateway_create(&managedGateway, NULL);
    TEST_ASSERT("IoTPManagedGateway_create: Create managedGateway with NULL config", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPManagedGateway_destroy(NULL);
    TEST_ASSERT("IoTPManagedGateway_create: Destroy NULL managedGateway handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    /* success tests */
    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("IoTPManagedGateway_create: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_create(&managedGateway, config);
    TEST_ASSERT("IoTPManagedGateway_create: Create managedGateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_destroy(managedGateway);
    TEST_ASSERT("IoTPManagedGateway_create: Destroy a valid managedGateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPManagedGateway_create: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: MQTT Log handler setup */
int testManagedGateway_setMQTTLogHandler(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedGateway *managedGateway = NULL;

    /* Error tests */
    rc = IoTPManagedGateway_setMQTTLogHandler(NULL, NULL);
    TEST_ASSERT("IoTPManagedGateway_setMQTTLogHandler: Invalid log handler", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("IoTPManagedGateway_setMQTTLogHandler: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_create(&managedGateway, config);
    TEST_ASSERT("IoTPManagedGateway_setMQTTLogHandler: Create managedGateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_setMQTTLogHandler(managedGateway, &MQTTTraceCallback);
    TEST_ASSERT("IoTPManagedGateway_setMQTTLogHandler: Valid log handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_connect(managedGateway);
    TEST_ASSERT("IoTPManagedGateway_setMQTTLogHandler: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);
    /* TEST_ASSERT("IoTPManagedGateway_setMQTTLogHandler: Verify loghandler", mqttLogCallbackActive == 1, "rcE=%d rcA=%d", mqttLogCallbackActiveExp, mqttLogCallbackActive); */
    rc = IoTPManagedGateway_destroy(managedGateway);
    TEST_ASSERT("IoTPManagedGateway_setMQTTLogHandler: Destroy a valid managedGateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPManagedGateway_setMQTTLogHandler: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: Device connect/disconnect/send */
int testManagedGateway_connect(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedGateway *managedGateway = NULL;
    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("IoTPManagedGateway_connect: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    IoTPConfig_readEnvironment(config);
    rc = IoTPManagedGateway_create(&managedGateway, config);
    TEST_ASSERT("IoTPManagedGateway_connect: Create managedGateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_connect(NULL);
    TEST_ASSERT("IoTPManagedGateway_connect: NULL", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPManagedGateway_connect(managedGateway);
    TEST_ASSERT("IoTPManagedGateway_connect: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_disconnect(managedGateway);
    TEST_ASSERT("IoTPManagedGateway_connect: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_destroy(managedGateway);
    TEST_ASSERT("IoTPManagedGateway_connect: Destroy a valid managedGateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPManagedGateway_connect: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: Send event - error cases */
int testManagedGateway_sendEventVal(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedGateway *managedGateway = NULL;

    rc = IoTPManagedGateway_sendEvent(managedGateway, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Invalid managedGateway object", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_create(&managedGateway, config);
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Create managedGateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPManagedGateway_sendEvent(managedGateway, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Invalid event ID", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPManagedGateway_sendEvent(managedGateway, "status", NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Invalid format", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPManagedGateway_sendEvent(managedGateway, "status", NULL, "json", -1, NULL);
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Invalid QoS=-1", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_INVALID_VALUE, rc);
    rc = IoTPManagedGateway_sendEvent(managedGateway, "status", NULL, "json", 3, NULL);
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Invalid QoS=3", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_INVALID_VALUE, rc);
    rc = IoTPManagedGateway_destroy(managedGateway);
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Destroy a valid managedGateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPManagedGateway_sendEventVal: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: ManagedGateway send event */
int testManagedGateway_sendEvent(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPManagedGateway *managedGateway = NULL;
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    int i = 0;

    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("testManagedGateway_sendEvent: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    /* get org id, and managedGateway token from environment */
    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("testManagedGateway_sendEvent: Read config from environment", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPManagedGateway_create(&managedGateway, config);
    TEST_ASSERT("testManagedGateway_sendEvent: Create managedGateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPManagedGateway_setMQTTLogHandler(managedGateway, &MQTTTraceCallback);
    TEST_ASSERT("testManagedGateway_sendEvent: Set MQTT Trace handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPManagedGateway_connect(managedGateway);
    TEST_ASSERT("testManagedGateway_sendEvent: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);

    for (i=0; i<2; i++) {
        rc = IoTPManagedGateway_sendEvent(managedGateway,"status","json", data , QoS0, NULL);
        TEST_ASSERT("testManagedGateway_sendEvent: Send event QoS0", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        sleep(5);
    }

    for (i=0; i<2; i++) {
        rc = IoTPManagedGateway_sendDeviceEvent(managedGateway,"iotc_test_devType1", "iotc_test_dev1", "status","json", data , QoS0, NULL);
        TEST_ASSERT("testManagedGateway_sendDeviceEvent: Send event QoS0", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        sleep(5);
    }

    rc = IoTPManagedGateway_disconnect(managedGateway);
    TEST_ASSERT("testManagedGateway_sendEvent: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    sleep(2);

    rc = IoTPManagedGateway_destroy(managedGateway);
    TEST_ASSERT("testManagedGateway_sendEvent: Destroy a valid managedGateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("testManagedGateway_sendEvent: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


int main(void)
{
    int rc = 0;
    int (*tests[])() = {testManagedGateway_create, testManagedGateway_setMQTTLogHandler, testManagedGateway_sendEventVal, testManagedGateway_connect, testManagedGateway_sendEvent};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Managed Gateway API Tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
    }

    testEnd("IBM IoT Platform Client: Managed Gateway API Tests", count);

    return rc;
}

