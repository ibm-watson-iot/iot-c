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
 * systemGateway_tests.c: IBM Watson IoT Platform C Client Gateway API system level tests
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


/* Tests: Gateway send event */
int test_sendEvent(void)
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPGateway *gateway = NULL;
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    int i = 0;

    rc = IoTPConfig_create(&config, "./wiotpgw.yaml");
    TEST_ASSERT("test_sendEvent: Create config object", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    /* get org id, and gateway token from environment */
    rc = IoTPConfig_readEnvironment(config);
    TEST_ASSERT("test_sendEvent: Read config from environment", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPGateway_create(&gateway, config);
    TEST_ASSERT("test_sendEvent: Create gateway with valid config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPGateway_setMQTTLogHandler(gateway, &MQTTTraceCallback);
    TEST_ASSERT("test_sendEvent: Set MQTT Trace handler", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPGateway_connect(gateway);
    TEST_ASSERT("test_sendEvent: Connect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    sleep(2);

    for (i=0; i<2; i++) {
        rc = IoTPGateway_sendEvent(gateway,"status","json", data , QoS0, NULL);
        TEST_ASSERT("test_sendEvent: Send event QoS0", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        sleep(5);
    }

    for (i=0; i<2; i++) {
        rc = IoTPGateway_sendDeviceEvent(gateway,"iotc_test_devType1", "iotc_test_dev1", "status","json", data , QoS0, NULL);
        TEST_ASSERT("test_sendDeviceEvent: Send event QoS0", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
        sleep(5);
    }

    rc = IoTPGateway_disconnect(gateway);
    TEST_ASSERT("test_sendEvent: Disconnect client", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    sleep(2);

    rc = IoTPGateway_destroy(gateway);
    TEST_ASSERT("test_sendEvent: Destroy a valid gateway handle", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);

    rc = IoTPConfig_clear(config);
    TEST_ASSERT("test_sendEvent: Clear Config", rc == IOTPRC_SUCCESS, "rcE=%d rcA=%d", IOTPRC_SUCCESS, rc);
    return rc;
}

int main(int argc, char** argv)
{
    int rc = 0;
    int (*tests[])() = {test_sendEvent};
    int i;
    int count = (int)TEST_COUNT(tests);

    testStart("IBM IoT Platform Client: Gateway system level tests", count);

    for (i = 0; i < count; i++) {
        printf("Run TestSuite:%d\n", i+1);
        tests[i]();
        printf("\n");
        sleep(2);
    }

    testEnd("IBM IoT Platform Client: Gateway system level tests", count);

    return rc;
}

