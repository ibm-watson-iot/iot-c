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
#include "iotp_gateway.h"

/*
 * validateGateway_tests.c: IBM Watson IoT Platform C Client Gateway API validation tests
 *
 * This file contains test functions to test validation of following APIs:
 * - IoTPGateway_create
 * - IoTPGateway_destroy
 * - IoTPGateway_setMQTTLogHandler
 * - IoTPGateway_connect
 * - IoTPGateway_disconnect
 * - IoTPGateway_sendEvent
 * - IoTPGateway_setCommandsHandler
 * - IoTPGateway_subscribeToCommands
 * - IoTPGateway_handleCommand
 * - IoTPGateway_unsubscribeFromCommands
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
int testGateway_create(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPGateway *gateway = NULL;

    /* error tests */
    rc = IoTPGateway_create(NULL, config);
    TEST_ASSERT("IoTPGateway_create: Create gateway with NULL gateway handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPGateway_create(&gateway, NULL);
    TEST_ASSERT("IoTPGateway_create: Create gateway with NULL config", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPGateway_destroy(NULL);
    TEST_ASSERT("IoTPGateway_create: Destroy NULL gateway handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    /* success tests */
    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("IoTPGateway_create: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_create(&gateway, config);
    TEST_ASSERT("IoTPGateway_create: Create gateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_destroy(gateway);
    TEST_ASSERT("IoTPGateway_create: Destroy a valid gateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPGateway_create: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: MQTT Log handler setup */
int testGateway_setMQTTLogHandler(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPGateway *gateway = NULL;

    /* Error tests */
    rc = IoTPGateway_setMQTTLogHandler(NULL, NULL);
    TEST_ASSERT("IoTPGateway_setMQTTLogHandler: Invalid log handler", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("IoTPGateway_setMQTTLogHandler: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_create(&gateway, config);
    TEST_ASSERT("IoTPGateway_setMQTTLogHandler: Create gateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_setMQTTLogHandler(gateway, &MQTTTraceCallback);
    TEST_ASSERT("IoTPGateway_setMQTTLogHandler: Valid log handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_connect(gateway);
    TEST_ASSERT("IoTPGateway_setMQTTLogHandler: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);
    /* TEST_ASSERT("IoTPGateway_setMQTTLogHandler: Verify loghandler", mqttLogCallbackActive == 1, "rcE=%d rcA=%d", mqttLogCallbackActiveExp, mqttLogCallbackActive); */
    rc = IoTPGateway_destroy(gateway);
    TEST_ASSERT("IoTPGateway_setMQTTLogHandler: Destroy a valid gateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPGateway_setMQTTLogHandler: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: Device connect/disconnect/send */
int testGateway_connect(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPGateway *gateway = NULL;
    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("IoTPGateway_connect: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    IoTPConfig_readEnvironment(config);
    rc = IoTPGateway_create(&gateway, config);
    TEST_ASSERT("IoTPGateway_connect: Create gateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_connect(NULL);
    TEST_ASSERT("IoTPGateway_connect: NULL", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPGateway_connect(gateway);
    TEST_ASSERT("IoTPGateway_connect: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_disconnect(gateway);
    TEST_ASSERT("IoTPGateway_connect: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_destroy(gateway);
    TEST_ASSERT("IoTPGateway_connect: Destroy a valid gateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPGateway_connect: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: Send event - error cases */
int testGateway_sendEventVal(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPGateway *gateway = NULL;

    rc = IoTPGateway_sendEvent(gateway, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPGateway_sendEventVal: Invalid gateway object", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("IoTPGateway_sendEventVal: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_create(&gateway, config);
    TEST_ASSERT("IoTPGateway_sendEventVal: Create gateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPGateway_sendEvent(gateway, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPGateway_sendEventVal: Invalid event ID", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPGateway_sendEvent(gateway, "status", NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPGateway_sendEventVal: Invalid format", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPGateway_sendEvent(gateway, "status", NULL, "json", -1, NULL);
    TEST_ASSERT("IoTPGateway_sendEventVal: Invalid QoS=-1", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_INVALID_VALUE, rc);
    rc = IoTPGateway_sendEvent(gateway, "status", NULL, "json", 3, NULL);
    TEST_ASSERT("IoTPGateway_sendEventVal: Invalid QoS=3", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_INVALID_VALUE, rc);
    rc = IoTPGateway_destroy(gateway);
    TEST_ASSERT("IoTPGateway_sendEventVal: Destroy a valid gateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPGateway_sendEventVal: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: Gateway send event */
int testGateway_sendEvent(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPGateway *gateway = NULL;
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    int i = 0;

    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("testGateway_sendEvent: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    /* get org id, and gateway token from environment */
    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("testGateway_sendEvent: Read config from environment", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPGateway_create(&gateway, config);
    TEST_ASSERT("testGateway_sendEvent: Create gateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPGateway_setMQTTLogHandler(gateway, &MQTTTraceCallback);
    TEST_ASSERT("testGateway_sendEvent: Set MQTT Trace handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPGateway_connect(gateway);
    TEST_ASSERT("testGateway_sendEvent: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);

    for (i=0; i<2; i++) {
        rc = IoTPGateway_sendEvent(gateway,"status","json", data , QoS0, NULL);
        TEST_ASSERT("testGateway_sendEvent: Send event QoS0", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        sleep(5);
    }

    for (i=0; i<2; i++) {
        rc = IoTPGateway_sendDeviceEvent(gateway,"iotc_test_devType1", "iotc_test_dev1", "status","json", data , QoS0, NULL);
        TEST_ASSERT("testGateway_sendEvent: Send event QoS0", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        sleep(5);
    }

    rc = IoTPGateway_disconnect(gateway);
    TEST_ASSERT("testGateway_sendEvent: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    sleep(2);

    rc = IoTPGateway_destroy(gateway);
    TEST_ASSERT("testGateway_sendEvent: Destroy a valid gateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("testGateway_sendEvent: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

int main(void)
{
    int rc = 0;
    int (*tests[])() = {testGateway_create, testGateway_setMQTTLogHandler, testGateway_sendEventVal, testGateway_connect, testGateway_sendEvent};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Gateway API Tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
    }

    testEnd("IBM IoT Platform Client: Gateway API Tests", count);

    return rc;
}

