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
#include "iotp_application.h"

/*
 * validateApplication_tests.c: IBM Watson IoT Platform C Client Application API validation tests
 *
 * This file contains test functions to test validation of following APIs:
 * - IoTPApplication_create
 * - IoTPApplication_destroy
 * - IoTPApplication_setMQTTLogHandler
 * - IoTPApplication_connect
 * - IoTPApplication_disconnect
 * - IoTPApplication_sendEvent
 * - IoTPApplication_sendCommand
 * - IoTPApplication_setEventHandler
 * - IoTPApplication_subscribeToEvents
 * - IoTPApplication_unsubscribeFromEvents
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
int testApplication_create(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPApplication *application = NULL;

    /* error tests */
    rc = IoTPApplication_create(NULL, config);
    TEST_ASSERT("IoTPApplication_create: Create application with NULL application handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPApplication_create(&application, NULL);
    TEST_ASSERT("IoTPApplication_create: Create application with NULL config", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPApplication_destroy(NULL);
    TEST_ASSERT("IoTPApplication_create: Destroy NULL application handle", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    /* success tests */
    rc = IoTPConfig_create(&config, "./wiotpapp.yaml");
    TEST_ASSERT("IoTPApplication_create: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_create(&application, config);
    TEST_ASSERT("IoTPApplication_create: Create application with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_destroy(application);
    TEST_ASSERT("IoTPApplication_create: Destroy a valid application handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPApplication_create: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: MQTT Log handler setup */
int testApplication_setMQTTLogHandler(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPApplication *application = NULL;

    /* Error tests */
    rc = IoTPApplication_setMQTTLogHandler(NULL, NULL);
    TEST_ASSERT("IoTPApplication_setMQTTLogHandler: Invalid log handler", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);

    rc = IoTPConfig_create(&config, "./wiotpapp.yaml");
    TEST_ASSERT("IoTPApplication_setMQTTLogHandler: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_create(&application, config);
    TEST_ASSERT("IoTPApplication_setMQTTLogHandler: Create application with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_setMQTTLogHandler(application, &MQTTTraceCallback);
    TEST_ASSERT("IoTPApplication_setMQTTLogHandler: Valid log handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_connect(application);
    TEST_ASSERT("IoTPApplication_setMQTTLogHandler: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    /* TEST_ASSERT("IoTPApplication_setMQTTLogHandler: Verify loghandler", mqttLogCallbackActive == 1, "rcE=%d rcA=%d", mqttLogCallbackActiveExp, mqttLogCallbackActive); */
    rc = IoTPApplication_destroy(application);
    TEST_ASSERT("IoTPApplication_setMQTTLogHandler: Destroy a valid application handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPApplication_setMQTTLogHandler: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}


/* Tests: Device connect/disconnect/send */
int testApplication_connect(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPApplication *application = NULL;
    rc = IoTPConfig_create(&config, "./wiotpapp.yaml");
    TEST_ASSERT("IoTPApplication_connect: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("IoTPApplication_connect: Read environment variables", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_create(&application, config);
    TEST_ASSERT("IoTPApplication_connect: Create application with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    // rc = IoTPApplication_connect(NULL);
    // TEST_ASSERT("IoTPApplication_connect: NULL", rc == IOTPRC_INVALID_HANDLE, "rcE=%d rcA=%d", IOTPRC_INVALID_HANDLE, rc);
    rc = IoTPApplication_connect(application);
    TEST_ASSERT("IoTPApplication_connect: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_disconnect(application);
    TEST_ASSERT("IoTPApplication_connect: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_destroy(application);
    TEST_ASSERT("IoTPApplication_connect: Destroy a valid application handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPApplication_connect: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

/* Tests: Send event - error cases */
int testApplication_sendEvent(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPApplication *application = NULL;

    rc = IoTPApplication_sendEvent(application, NULL, NULL, NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPApplication_sendEvent: Invalid application object", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPConfig_create(&config, "./wiotpapp.yaml");
    TEST_ASSERT("IoTPApplication_sendEvent: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_create(&application, config);
    TEST_ASSERT("IoTPApplication_sendEvent: Create application with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPApplication_sendEvent(application, "type1", "id1", NULL, NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPApplication_sendEvent: Invalid event ID", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPApplication_sendEvent(application, "type1", "id1", "status", NULL, NULL, 0, NULL);
    TEST_ASSERT("IoTPApplication_sendEvent: Invalid format", rc == IOTPRC_ARGS_NULL_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPApplication_sendEvent(application, "type1", "id1", "status", NULL, "json", -1, NULL);
    TEST_ASSERT("IoTPApplication_sendEvent: Invalid QoS=-1", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPApplication_sendEvent(application, "type1", "id1", "status", NULL, "json", 3, NULL);
    TEST_ASSERT("IoTPApplication_sendEvent: Invalid QoS=3", rc == IOTPRC_ARGS_INVALID_VALUE, "rcE=%d rcA=%d", IOTPRC_ARGS_NULL_VALUE, rc);
    rc = IoTPApplication_destroy(application);
    TEST_ASSERT("IoTPApplication_sendEvent: Destroy a valid application handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    rc = IoTPConfig_clear(config);
    TEST_ASSERT("IoTPApplication_sendEvent: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

int main(int argc, char** argv)
{
    int rc = 0;
    int (*tests[])() = {testApplication_create, testApplication_setMQTTLogHandler, testApplication_sendEvent, testApplication_connect};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Application API Tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
        sleep(5);
    }

    testEnd("IBM IoT Platform Client: Application API Tests", count);

    return rc;
}

