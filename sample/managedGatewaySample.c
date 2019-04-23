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
 * Contributors:
 *    Ranjan Dasgupta - Initial drop of gatewaySample.c
 * 
 *******************************************************************************/

/*
 * This sample shows how-to develop a managed gateway code using Watson IoT Platform
 * iot-c managed gateway client library, connect and interact with Watson IoT Platform Service.
 * 
 * This sample includes the function/code snippets to perform the following actions:
 * - Initiliaze client library
 * - Configure managed gateway from configuration parameters specified in a configuration file
 * - Set client logging
 * - Enable error handling routines
 * - Send gateway events to WIoTP service
 * - Receive and process commands from WIoTP service
 * - Make gateway as a managed gateway
 * - Receive managed gateway action and process it
 *
 * SYNTAX:
 * managedGatewaySample --config <config_file_path>
 *
 * Pre-requisite:
 * 1. This sample requires a managed gateway configuration file. 
 *    Refer to "Managed Gateway Configuration File" section of iot-c docs for details.
 * 2. Register type and id (specified in the configuration file) with IBM Watson IoT Platform service.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>

/* Include header file of IBM Watson IoT platform C Client for managed gateways */ 
#include "iotp_managedGateway.h"

char *configFilePath = NULL;
volatile int interrupt = 0;
char *progname = "managedGatewaySample";

/* Usage text */
void usage(void) {
    fprintf(stderr, "Usage: %s --config config_file_path\n", progname);
    exit(1);
}

/* Signal handler - to support CTRL-C to quit */
void sigHandler(int signo) {
    signal(SIGINT, NULL);
    fprintf(stdout, "Received signal: %d\n", signo);
    interrupt = 1;
}

/* Get and process command line options */
void getopts(int argc, char** argv)
{
    int count = 1;

    while (count < argc)
    {
        if (strcmp(argv[count], "--config") == 0)
        {
            if (++count < argc)
                configFilePath = argv[count];
            else
                usage();
        }
        count++;
    }
}

/* 
 * Gateway command callback function
 * Gateway developers can customize this function based on their use case
 * to handle gateway commands sent by WIoTP.
 * Set this callback function using API setCommandHandler().
 */
void  gatewayCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received gateway command:\n");
    fprintf(stdout, "Type=%s ID=%s CommandName=%s Format=%s Len=%d\n", type, id, commandName, format, (int)payloadSize);
    fprintf(stdout, "Payload: %s\n", (char *)payload);

    /* Gateway developers - add your custom code to process gateway commands */
}

void logCallback (int level, char * message)
{
    if ( level > 0 )
        fprintf(stdout, "%s\n", message? message:"NULL");
    fflush(stdout);
}

void MQTTTraceCallback (int level, char * message)
{
    if ( level > 0 )
        fprintf(stdout, "%s\n", message? message:"NULL");
    fflush(stdout);
}

/*
 * Managed Gateway DM callback function
 */
void DMActionCallbak(IoTP_DMAction_type_t type, char *reqId, void *payload, size_t payloadlen) {
    char *pl = NULL;
    fprintf(stdout, "DM Requst ID: %s\n", reqId);
    switch(type) {
        case IoTP_DMResponse:
            pl = (char *)payload;
            pl[payloadlen] = 0;
            fprintf(stdout, "Received DM Action response\n");
            fprintf(stdout, "Payload: %s\n", payload?(char *)pl:"");
            /* Add code for your device */
            break;

        case IoTP_DMFactoryReset:
            fprintf(stdout, "Start factory reset action\n");
            /* Add code for your device */
            break;

        case IoTP_DMReboot:
            fprintf(stdout, "Start device reboot action\n");
            /* Add code for your device */
            break;

        case IoTP_DMFirmwareDownload:
            fprintf(stdout, "Start Firmware download action\n");
            /* Add code for your device */
            break;

        case IoTP_DMFirmwareUpdate:
            fprintf(stdout, "Start Firmware Update action\n");
            /* Add code for your device */
            break;

        default:
            fprintf(stdout, "Can not handle DM action: %d\n", type);
            break;
    }
}


/* Main program */
int main(int argc, char *argv[])
{
    int rc = 0;

    /* 
     * DEV_NOTES:
     * Specifiy variable for WIoT client object 
     */
    IoTPConfig *config = NULL;
    IoTPManagedGateway *managedGateway = NULL;

    /* check for args */
    if ( argc < 2 )
        usage();

    /* Set signal handlers */
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    /* get argument options */
    getopts(argc, argv);

    /* Set IoTP Client log handler */
    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stdout);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set IoTP Client log handler: rc=%d\n", rc);
    }

    /* Create IoTPConfig object using configuration options defined in the configuration file. */
    rc = IoTPConfig_create(&config, configFilePath);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to initialize configuration: rc=%d\n", rc);
        exit(1);
    }

    /* Create IoTPManagedGateway object */
    rc = IoTPManagedGateway_create(&managedGateway, config);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to configure IoTP managed gateway: rc=%d\n", rc);
        exit(1);
    }

    /* Set MQTT Trace handler */
    rc = IoTPManagedGateway_setMQTTLogHandler(managedGateway, &MQTTTraceCallback);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set MQTT Trace handler: rc=%d\n", rc);
    }

    /* Invoke connection API IoTPManagedGateway_connect() to connect to WIoTP. */
    rc = IoTPManagedGateway_connect(managedGateway);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to connect to Watson IoT Platform: rc=%d\n", rc);
        exit(1);
    }

    /*
     * Set managed gateway command callback using API IoTPManagedGateway_setCommandHandler().
     * Refer to gatewayCommandCallback() function DEV_NOTES for details on
     * how to process gateway commands received from WIoTP.
     */
    IoTPManagedGateway_setCommandHandler(managedGateway, gatewayCommandCallback);

    /* set managed gateway attribute */
    rc = IoTPManagedGateway_setAttribute(managedGateway, "lifetime", "3600");
    rc |= IoTPManagedGateway_setAttribute(managedGateway, "deviceActions", "1");
    rc |= IoTPManagedGateway_setAttribute(managedGateway, "firmwareActions", "1");
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to set managed gateway attributes: rc=%d\n", rc);
        goto device_exit;
    }

    /*
     * Set callback to process device management actions
     */
    rc = IoTPManagedGateway_setActionHandler(managedGateway, IoTP_DMActions, DMActionCallbak);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to set gateway management action callback: rc=%d\n", rc);
        goto device_exit;
    }

    /* Make this gateway a managed gateway */
    rc = IoTPManagedGateway_manage(managedGateway);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to make gateway a managed gateway: rc=%d\n", rc);
        goto device_exit;
    }

    /*
     * Invoke gateway command subscription API IoTPManagedGateway_subscribeToCommands().
     * The arguments for this API are commandName, format, QoS
     * If you want to subscribe to all commands of any format, set commandName and format to "+"
     */
    char *commandName = "+";
    char *format = "+";
    IoTPManagedGateway_subscribeToCommands(managedGateway, commandName, format);


    /* Use IoTPManagedGateway_sendEvent() API to send gateway events to Watson IoT Platform. */

    /* Sample event - this sample gateway will send this event once in every 10 seconds. */
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";

    while(!interrupt)
    {
        fprintf(stdout, "Send status event\n");
        rc = IoTPManagedGateway_sendEvent(managedGateway,"status",data, "json", QoS0, NULL);
        fprintf(stdout, "RC from publishEvent(): %d\n", rc);
        sleep(10);
    }

    fprintf(stdout, "Received a signal - exiting publish event cycle.\n");

    /* Make this gateway an unmanaged gateway */
    rc = IoTPManagedGateway_unmanage(managedGateway, NULL);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to make device an unmanaged gateway: rc=%d\n", rc);
    }

device_exit:

    /* Disconnect gateway */
    rc = IoTPManagedGateway_disconnect(managedGateway);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "ERROR: Failed to disconnect from  Watson IoT Platform: rc=%d\n", rc);
        exit(1);
    }

    /* Destroy client */
    IoTPManagedGateway_destroy(managedGateway);

    /* Clear configuration */
    IoTPConfig_clear(config);

    return 0;

}

