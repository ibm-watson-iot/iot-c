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
 * systemDevice_tests.c: IBM Watson IoT Platform C Client Device API system level tests
 *
 * - Send events
 * - Receive commands
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


/* Tests: Device send event */
int test_sendEvent(void)
{
    int rc = IoTP_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    int i = 0;

    rc = IoTPConfig_create(&config, "./iotpclient.cfg");
    TEST_ASSERT("test_sendEvent: Create config object", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    /* get org id, and device token from environment */
    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("test_sendEvent: Read config from environment", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPDevice_create(&device, config);
    TEST_ASSERT("test_sendEvent: Create device with valid config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    rc = IoTPDevice_connect(device);
    TEST_ASSERT("test_sendEvent: Connect client", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPDevice_sendEvent(device,"status","json", data , QoS0, NULL);
    TEST_ASSERT("test_sendEvent: Send event QoS0", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPDevice_disconnect(device);
    TEST_ASSERT("test_sendEvent: Disconnect client", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    sleep(2);

    rc = IoTPDevice_destroy(device);
    TEST_ASSERT("test_sendEvent: Destroy a valid device handle", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("test_sendEvent: Clear Config", rc == IoTP_SUCCESS, "rcE=%d rcA=%d", IoTP_SUCCESS, rc);
    return rc;
}

int main(int argc, char** argv)
{
    int rc = 0;
    int (*tests[])() = {test_sendEvent};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Device system level tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
        sleep(2);
    }

    testEnd("IBM IoT Platform Client: Device system level tests", count);

    return rc;
}

