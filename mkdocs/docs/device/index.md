# Device SDK

The device SDK includes a shared library that exposes functions to build device client that 
runs on IoT devices.

- `libwiotp-sdk-device-as.so`

Support APIs for creating a device client and managing connection to the Watson IoT Platform service:

- `IoTPConfig_create()`
- `IoTPDevice_create()`
- `IoTPDevice_connect()`

Support APIs for working with events and commands: 

- `IoTPDevice_sendEvent()`
- `IoTPDevice_setEventCallback()`
- `IoTPDevice_setCommandsHandler()`
- `IoTPDevice_subscribeToCommands()`


## Configuration

Device configuration is passed to the client via the `IoTPConfig` object when you create 
the client instance or handle `IoTPDevice`. See the [configure devices](config.md) section 
for full details of all available options to configure a client instance.

The config handle `IoTPConfig` created using `IoTPConfig_create()` must be cleared 
using `IoTPConfig_clear()` to avoid handle leak. The device handle `IoTPDevice` created 
using `IoTPDevice_create()` must be destroyed using `IoTPDevice_destory()` to avoid handle leak.


## Connectivity

`IoTPDevice_connect()` & `IoTPDevice_disconnect()` APIs are used to manage the MQTT connection
to the Watson IoT Platform service that allows the device to handle commands and publish events.

!!! Tip
    Though there are no restrictions on how many device clients, a device application can create, it is a good practice to not to create many client handles and connect to the Watson IoT platform service, to limit the number of connections to the Watson IoT Platforrm service, and reduce load on the Watson IoT Platform service.


## Publishing Events

Device can provide various types of information, for example data collected by the device
(telemetry data), current status of the device etc. Events are the mechanism by which devices 
publish data to the Watson IoT Platform service. The device controls the content of the event 
and assigns a name for each event that it sends.

When an event is received by Watson IoT Platform service, the credentials of the received event identify
the sending device, which means that a device cannot impersonate another device.

Events can be published with any of the three quality of service (QoS) levels that are defined
by the MQTT protocol. By default, events are published with a QoS level of 0.

Optionally you can send MQTT V5 properties. For details on MQTT V5 properties, refer to 
<a href="https://www.eclipse.org/paho/files/mqttdoc/MQTTAsync/html/struct_m_q_t_t_properties.html" target="_blank">MQTTProperty</a>.

To send events, you can use `IoTPDevice_sendEvent()` API. This API takes upto 6 arguments:

- `device` Client handle
- `event` Name of this event
- `msgFormat` Format of the data for this event
- `data` Data for this event
- `qos` MQTT quality of service level to use (`0`, `1`, or `2`)
- `props` MQTT V5 properties

You can set a callback to get send event response using `IoTPDevice_setEventCallback()` API. 
The callback returns success and failure responses. The success respose has different implications depending
on the level of qos used to publish the event:

- qos 0: the client has asynchronously begun to send the event
- qos 1 and 2: the client has confirmation of delivery from the platform


## Handling Commands

A device client can susbcribe to a command using `IoTPDevice_subscribeToCommands()` API.
To process specific commands, you need to register a command callback method, using `IoTPDevice_setCommandHandler()`.

The messages are returned as an instance of the `Command` class with the following attributes:

- `type`: Client type ID
- `id`: Client ID
- `command`: Identifies the command
- `format`: Format that the command was encoded in, for example `json`
- `payload`: Data for the payload
- `payloadlen`: Size of the payload buffer



## Sample

### Sample configuration file

```yaml
identity:
    orgId: org1id
    typeId: testDeviceType
    deviceId: testDevice
auth:
    token: testPassword
```

### Sample code

```
#include <iotp_device.h>

/* Device command callback function. Device developers can customize this function for their use case. */
void  deviceCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received device command. Type=%s ID=%s CommandName=%s Format=%s Len=%d\n",
        type?type:"", id?id:"", commandName?commandName:"", format?format:"", (int)payloadSize);
    /* Device developers - add your custom code to process device command and its payload */
}

/* Device event callback function. */
void eventCallback(char *id, int rc, void *success void *failure) {
    fprintf("Event callback response for client: %s\n", id?id:"");
    if ( rc == IOTPRC_SUCCESS ) {
        MQTTAsync_successData5 *response = success;
        if ( response ) {
            fprintf("Success response code:%d response_message:%s\n", response->code, response->message);
        } else {
            fprintf("Event is published.\n");
        }
    } else {
        MQTTAsync_failureData5 *response = failure;
        if ( response ) {
            fprintf("Failure response code:%d response_message:%s\n", response->code, response->message);
        } else {
            fprintf("Failed to publish event.\n");
        }
    }
}

int main(int argc, char *argv[])
{
    int rc = IOTPRC_SUCCESS;
    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;

    /*******************************************************/
    /****** Create device client instance and connect ******/
    /*******************************************************/
    /* Create IoTPConfig object, use it to create IoTPDevice handle, and connect client */
    rc = IoTPConfig_create(&config, "configFile.yaml");
    if ( rc == IOTPRC_SUCCESS ) {
        rc = IoTPDevice_create(&device, config);
        if ( rc == IOTPRC_SUCCESS ) {
            rc = IoTPDevice_connect(device);
        }
    }
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "Failed to create device client: rc=%d reason=%s\n", rc, IOTPRC_toString(rc));
        exit(1);
    }


    /******************************/
    /****** Process commands ******/
    /******************************/
    /* Set device command callback for all commands and subscribe to all commands in any supported format */
    rc = IoTPDevice_setCommandsHandler(device, deviceCommandCallback);
    if ( rc == IOTPRC_SUCCESS ) 
        rc = IoTPDevice_subscribeToCommands(device, "+", "+");
    }
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "Failed to set command handler or subscribe to commands. rc=%d reason:%s\n", rc, IOTPRC_toString(rc));
    }


    /****************************/
    /****** Publish events ******/
    /****************************/
    /* Set event callback to process event response and send event */
    rc = IoTPDevice_setEventCallback(device, eventCallback);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "Failed to set event callback. rc=%d reason:%s\n", rc, IOTPRC_toString(rc));
    }
    /* Set MQTT V5 properties */
    MQTTProperties *properties = (MQTTProperties *)malloc(sizeof MQTTProperties);
    MQTTProperty   property;
    property.identifier = MQTTPROPERTY_CODE_USER_PROPERTY;
    property.value.data.data = "user defined property";
    property.value.data.len = (int)strlen(property.value.data.data);
    property.value.value.data = "user defined property value";
    property.value.value.len = (int)strlen(property.value.value.data);
    rc = MQTTProperties_add(properties, &property);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf("Failed to set MQTT V5 properties. rc=%d reason:%s\n", rc, IOTPRC_toString(rc));
    }
    /* Set event data */
    char *data = "{\"d\" : {\"SensorID\": \"Test\", \"Reading\": 7 }}";
    rc = IoTPDevice_sendEvent(device,"status", data, "json", QoS0, properties);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf("Failed to publish event. rc=%d\n", rc);
    } else {
        /* wait for sometime to publish data */
        sleep(5);
    }


    /*********************/
    /****** Cleanup ******/
    /*********************/
    /* Disconnect, and cleaup */
    rc = IoTPDevice_disconnect(device);
    if ( rc == IOTPRC_SUCCESS ) {
        /* Destroy client */
        rc = IoTPDevice_destroy(device);
        if ( rc == IOTPRC_SUCCESS ) {
            /* Clear configuration */
            rc = IoTPConfig_clear(config);
        }
    }
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf("Failed to disconnect or clean handles. rc=%d reason:%s\n", rc, IOTPRC_toString(rc));
    }

    return rc;
}

```


## Device SDK Reference

For device configuration APIs, refer to <a href="https://ibm-watson-iot.github.io/iot-c/reference/iotp_config_8h.html" target="_iotsdkrefernce">Configuration Refernce Documentation</a>

For device client APIs, refer to <a href="https://ibm-watson-iot.github.io/iot-c/reference/iotp_device_8h.html" target="_iotsdkrefernce">Device Client Refernce Documentation</a>

For Return and error codes by C Client SDK, refer to <a href="https://ibm-watson-iot.github.io/iot-c/reference/iotp_rc_8h.html" target="_iotsdkrefernce">Return Code Refernce Documentation</a>


