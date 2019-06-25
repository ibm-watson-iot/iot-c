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
 *    Ranjan Dasgupta - Initial drop of psutilApp.c
 * 
 *******************************************************************************/

/*
 * This sample device code to send system utilization to Watson IoT Platform.
 * This sample code collects and sends the following data from the device:
 * - CPU utilization (in percent)
 * - Total available Memory (KB)
 * - Memory used by psutil process (KB)
 * - Outbound network utilization across all network interfaces (KB/s)
 * - Inbound network utilization across all network interfaces (KB/s)
 *
 * SYNTAX:
 * deviceSample --config <config_file_path>
 *
 */

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/timeb.h>


/* Include header file of IBM Watson IoT platform C Client for devices */ 
#include "iotp_device.h"

#define BUFFERSIZE (1024)

volatile int interrupt = 0;
char *progname = "psutilApp";
int quickstart = 0;
int useEnv = 1;
int testCycle = 0;
char defaultDeviceId[8];
long diskread = 0;
long diskwrite = 0;
u_int64_t netdown = 0;
u_int64_t netup = 0;
double cpu = 0.0;
float mem = 0.0;
char hostname[1024];

/* Usage text */
void usage(void) {
    fprintf(stderr, "Usage: %s --quickstart\n", progname);
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
        if (strcmp(argv[count], "--quickstart") == 0) {
            quickstart = 1;
        }
        count++;
    }
}

/* 
 * Device command callback function
 * Device developers can customize this function based on their use case
 * to handle device commands sent by WIoTP.
 * Set this callback function using API setCommandHandler().
 */
void  deviceCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received device command:\n");
    fprintf(stdout, "Type=%s ID=%s CommandName=%s Format=%s Len=%d\n", 
        type?type:"", id?id:"", commandName?commandName:"", format?format:"", (int)payloadSize);
    fprintf(stdout, "Payload: %s\n", payload?(char *)payload:"");

    /* Device developers - add your custom code to process device commands */
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

/* psutil functions */
/* Get disk I/O rate */
int getDiskRate(void)
{
    long long megabytes,j,i;
    FILE *fp;
    char buffer[BUFFERSIZE];
    struct timeb before,after;
    long long cycle = 4 * 1014 * 1024;

    for (j=0 ; j < BUFFERSIZE ;j++)
        buffer[j]='X';

    fp = fopen("tempfile.out", "w");
    ftime(&before);
    for (i=0; i < cycle; i++)
        fwrite(buffer, BUFFERSIZE, 1, fp);
    ftime(&after);
    long timetowrite = after.time - before.time;
    if (timetowrite > 0)
        diskwrite = ((cycle * BUFFERSIZE)/timetowrite) / 1024;
    fprintf(stdout, "disk.write: %ld (KB/s)\n", diskwrite);
    fclose(fp);

    fp = fopen("tempfile.out", "r");
    ftime(&before);
    for (i=0; i < cycle; i++)
        fread(buffer, BUFFERSIZE, 1, fp);
    ftime(&after);
    long timetoread=after.time-before.time;
    if (timetoread > 0)
        diskread = ((cycle * BUFFERSIZE)/timetoread) / 1024;
    fprintf(stdout, "disk.read: %ld (KB/s)\n", diskread);

    fclose(fp);
    unlink("tempfile.out");
    return 0;
}

/* Network I/O rate */
int getNetworkStats(void)
{
    int mib[] = {
        CTL_NET,
        PF_ROUTE,
        0,
        0,
        NET_RT_IFLIST2,
        0
    };
    size_t len;
    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        printf("sysctl error: %d\n", errno);
        return 1;
    }
    char *buf = (char *)malloc(len);
    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        printf("sysctl error: %d\n", errno);
        return 1;
    }
    char *lim = buf + len;
    char *next = NULL;
    u_int64_t totalibytes = 0;
    u_int64_t totalobytes = 0;
    for (next = buf; next < lim; ) {
        struct if_msghdr *ifm = (struct if_msghdr *)next;
        next += ifm->ifm_msglen;
        if (ifm->ifm_type == RTM_IFINFO2) {
            struct if_msghdr2 *if2m = (struct if_msghdr2 *)ifm;
            netdown += if2m->ifm_data.ifi_ibytes;
            netup += if2m->ifm_data.ifi_obytes;
        }
    }
    netdown = netdown / 1024;
    netup = netup / 1024;
    fprintf(stdout, "network.up: %qu (KB/s)\n", netup);
    fprintf(stdout, "network.down: %qu (KB/s)\n", netdown);
    return 0;
}

/* Get CPU usage percent */
int getCpuUsage(void)
{
    double loadavg[3];
    double sum = 0;
    int i = 0;

    /* Get CPU usage */
    if ( getloadavg(loadavg, 3) < 0 ) {
        printf("cpu: 0 (percent)\n");
        return 1;
    } else {
        for (i=0; i<3; i++) {
            sum += loadavg[i];
        }
        cpu = sum / 3;
        fprintf(stdout, "cpu: %f (percent)\n", cpu);
    }
    return 0;
}

/* Get memory usage percent */
int getMemUsage(void)
{
    struct rusage usage = {0};
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    long totalMem = (pages * page_size)/1024;

    if ( getrusage(RUSAGE_SELF, &usage) < 0 ) {
        printf("mem: 0 (percent)\n");
        return 1;
    } else {
        long memSelf = usage.ru_maxrss;
        mem = ( (float) memSelf / totalMem * 100 );
        printf("mem: %f (percent)\n", mem);
    }

    return 0;
}


/* Main program */
int main(int argc, char *argv[])
{
    int rc = 0;
    int cycle = 0;
    char UUID[40];

    /* 
     * DEV_NOTES:
     * Specifiy variable for WIoT client object 
     */
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;

    /* check for args */
    if ( argc < 2 )
        usage();

    /* Set signal handlers */
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    /* get argument options */
    getopts(argc, argv);

    /* Get hostname */
    gethostname(hostname, 1024);
 
    /* Generate random device id for quickstart */
    iotp_utils_generateUUID(UUID);
    sscanf(UUID, "%8s", defaultDeviceId);

    /* Set IoTP Client log handler */
    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stderr);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set IoTP Client log handler: rc=%d\n", rc);
        exit(1);
    }

    /* Create IoTPConfig object */
    rc = IoTPConfig_create(&config, NULL);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to initialize configuration: rc=%d\n", rc);
        exit(1);
    }

    /* set default configuration for quickstart */
    if ( quickstart == 1 ) {
        IoTPConfig_setProperty(config, IoTPConfig_identity_orgId, "quickstart");
        IoTPConfig_setProperty(config, IoTPConfig_identity_typeId, "iotcpsutil");
        IoTPConfig_setProperty(config, IoTPConfig_identity_deviceId, defaultDeviceId);
        IoTPConfig_setProperty(config, IoTPConfig_options_logLevel, "ERROR");
        fprintf(stdout, "\nWelcome to IBM Watson IoT Platform Quickstart, view a vizualization of live data from this device at the URL below:\n");
        fprintf(stdout, "https://quickstart.internetofthings.ibmcloud.com/#/device/%s/sensor/\n\n", defaultDeviceId);
    } else {
        /* read additional config from environment */
        IoTPConfig_readEnvironment(config);
    } 

    /* Create IoTPDevice object */
    rc = IoTPDevice_create(&device, config);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to configure IoTP device: rc=%d\n", rc);
        exit(1);
    }

    /* Set MQTT Trace handler */
    rc = IoTPDevice_setMQTTLogHandler(device, &MQTTTraceCallback);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set MQTT Trace handler: rc=%d\n", rc);
    }

    /* Invoke connection API IoTPDevice_connect() to connect to WIoTP. */
    rc = IoTPDevice_connect(device);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to connect to Watson IoT Platform: rc=%d reason: %s\n", rc, IOTPRC_toString(rc));
        exit(1);
    }

    /* Use IoTPDevice_sendEvent() API to send device events to Watson IoT Platform. */
    fprintf(stdout, "name: %s\n", hostname);

    /* Sample event format */
    char eventBuffer[1024];
    char *evtformat = "{\"name\":\"%s\",\"cpu\":%f,\"mem\":%f,\"network\":{\"up\":%qu,\"down\":%qu},\"disk\":{\"read\":%ld,\"write\":%ld}}";

    while(!interrupt)
    {
        /* Set system stats */
        getCpuUsage();
        getMemUsage();
        getNetworkStats();
        getDiskRate();
        snprintf(eventBuffer, 1024, evtformat, hostname, cpu, mem, netup, netdown, diskread, diskwrite);

        fprintf(stdout, "Send status event\n");
        rc = IoTPDevice_sendEvent(device,"psutil", eventBuffer, "json", QoS0, NULL);
        fprintf(stdout, "RC from publishEvent(): %d\n", rc);

        if ( testCycle > 0 ) {
            cycle += 1;
            if ( cycle >= testCycle ) {
                break;
            }
        }
        sleep(10);
    }

    fprintf(stdout, "Publish event cycle is complete.\n");

    /* Disconnect device */
    rc = IoTPDevice_disconnect(device);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "ERROR: Failed to disconnect from  Watson IoT Platform: rc=%d\n", rc);
        exit(1);
    }

    /* Destroy client */
    IoTPDevice_destroy(device);

    /* Clear configuration */
    IoTPConfig_clear(config);

    return 0;

}

