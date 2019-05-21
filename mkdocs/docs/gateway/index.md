# Gateway SDK

The gateway SDK includes a shared library that exposes functions to build gateway client that 
runs on IoT gateways.

- `libwiotp-sdk-gateway-as.so`

Support APIs for creating a gateway client and managing connection to the Watson IoT Platform service:

- `IoTPConfig_create()`
- `IoTPGateway_create()`
- `IoTPGateway_connect()`

Support APIs for working with events and commands: 

- `IoTPGateway_sendEvent()`
- `IoTPGateway_setEventCallback()`
- `IoTPGateway_setCommandsHandler()`
- `IoTPGateway_subscribeToCommands()`
- `IoTPGateway_sendDeviceEvent()`
- `IoTPGateway_subscribeToDeviceCommands()`

## Configuration

Gateway configuration is passed to the client via the `IoTPConfig` object when you create 
the client instance or handle `IoTPGateway`. See the [configure gateways](config.md) section 
for full details of all available options to configure a client instance.

The config handle `IoTPConfig` created using `IoTPConfig_create()` must be cleared 
using `IoTPConfig_clear()` to avoid handle leak. The gateway handle `IoTPGateway` created 
using `IoTPGateway_create()` must be destroyed using `IoTPGateway_destory()` to avoid handle leak.


## Connectivity

`IoTPGateway_connect()` & `IoTPGateway_disconnect()` APIs are used to manage the MQTT connection
to the Watson IoT Platform service that allows the gateway to handle commands and publish events.

!!! Tip
    Though there are no restrictions on how many gateway clients, a gateway application can create, it is a good practice to not to create many client handles and connect to the Watson IoT platform service, to limit the number of connections to the Watson IoT Platforrm service, and reduce load on the Watson IoT Platform service.


## Publishing Events

Gateway can provide various types of information, for example data collected by the gateway
(telemetry data), current status of the gateway etc. Events are the mechanism by which gateways 
publish data to the Watson IoT Platform service. The gateway controls the content of the event 
and assigns a name for each event that it sends.

When an event is received by Watson IoT Platform service, the credentials of the received event identify
the sending gateway, which means that a gateway cannot impersonate another gateway.

Events can be published with any of the three quality of service (QoS) levels that are defined
by the MQTT protocol. By default, events are published with a QoS level of 0.

Optionally you can send MQTT V5 properties. For details on MQTT V5 properties, refer to 
<a href="https://www.eclipse.org/paho/files/mqttdoc/MQTTAsync/html/struct_m_q_t_t_properties.html" target="_blank">MQTTProperty</a>.

To send events, you can use `IoTPGateway_sendEvent()` API. This API takes upto 6 arguments:

- `gateway` Client handle
- `event` Name of this event
- `msgFormat` Format of the data for this event
- `data` Data for this event
- `qos` MQTT quality of service level to use (`0`, `1`, or `2`)
- `props` MQTT V5 properties

You can set a callback to get send event response using `IoTPGateway_setEventCallback()` API. 
The callback returns success and failure responses. The success respose has different implications depending
on the level of qos used to publish the event:

- qos 0: the client has asynchronously begun to send the event
- qos 1 and 2: the client has confirmation of delivery from the platform

## Handling Commands

A gateway client can susbcribe to a command using `IoTPGateway_subscribeToCommands()` API.
To process specific commands, you need to register a command callback method, using `IoTPGateway_setCommandHandler()`.

The messages are returned as an instance of the `Command` class with the following attributes:

- `type`: Client type ID
- `id`: Client ID
- `command`: Identifies the command
- `format`: Format that the command was encoded in, for example `json`
- `payload`: Data for the payload
- `payloadlen`: Size of the payload buffer

## Auto-regiter Devices

Gatway devices can automatically register devices that are connected to them. When a gateway publishes a message or subscribes to a topic on behalf of an unregistered device, that device is automatically registered.

!!! tip
    Registration requests from gateway devices are throttled to 128 pending requests at a time. Attempting to connect many new devices might cause a delay in the registration of the devices through the gateway.

!!! warning
    If the gateway fails to register a device automatically, it does not attempt to register that device again for a short time. Any messages or subscriptions from the failed device are dropped during that time.

### Publish Device Event Through Gateway

To send device events, you can use `IoTPGateway_sendDeviceEvent()` API. This API takes upto 8 arguments:

- `gateway` Client handle
- `typeId` Device type Id
- `deviceId` Device Id
- `event` Name of this event
- `msgFormat` Format of the data for this event
- `data` Data for this event
- `qos` MQTT quality of service level to use (`0`, `1`, or `2`)
- `props` MQTT V5 properties

### Handling Device Commands

A gateway client can susbcribe to device command connecting thrpugh the gateway client
using `IoTPGateway_subscribeToDeviceCommands()` API. This API takes 5 arguments:

- `gateway` Client handle
- `typeId` Device type Id
- `deviceId` Device Id
- `event` Name of this event
- `msgFormat` Format of the data for this event

To process specific commands, you need to register a command callback method, using `IoTPGateway_setCommandHandler()`.

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
    typeId: testGatewayType
    gatewayId: testGateway
auth:
    token: testPassword
```

### Sample code

```
#include <iotp_gateway.h>

/* Gateway command callback function. Gateway developers can customize this function for their use case. */
void  gatewayCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received gateway command. Type=%s ID=%s CommandName=%s Format=%s Len=%d\n",
        type?type:"", id?id:"", commandName?commandName:"", format?format:"", (int)payloadSize);
    /* Gateway developers - add your custom code to process gateway command and its payload */
}

/* Gateway event callback function. */
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
    IoTPGateway *gateway = NULL;

    /*******************************************************/
    /****** Create gateway client instance and connect ******/
    /*******************************************************/
    /* Create IoTPConfig object, use it to create IoTPGateway handle, and connect client */
    rc = IoTPConfig_create(&config, "configFile.yaml");
    if ( rc == IOTPRC_SUCCESS ) {
        rc = IoTPGateway_create(&gateway, config);
        if ( rc == IOTPRC_SUCCESS ) {
            rc = IoTPGateway_connect(gateway);
        }
    }
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "Failed to create gateway client: rc=%d reason=%s\n", rc, IOTPRC_toString(rc));
        exit(1);
    }


    /******************************/
    /****** Process commands ******/
    /******************************/
    /* Set gateway command callback for all commands and subscribe to all commands in any supported format */
    rc = IoTPGateway_setCommandsHandler(gateway, gatewayCommandCallback);
    if ( rc == IOTPRC_SUCCESS ) 
        rc = IoTPGateway_subscribeToCommands(gateway, "+", "+");
    }
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "Failed to set command handler or subscribe to commands. rc=%d reason:%s\n", rc, IOTPRC_toString(rc));
    }


    /****************************/
    /****** Publish events ******/
    /****************************/
    /* Set event callback to process event response and send event */
    rc = IoTPGateway_setEventCallback(gateway, eventCallback);
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
    rc = IoTPGateway_sendEvent(gateway,"status", data, "json", QoS0, properties);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf("Failed to publish event. rc=%d\n", rc);
    } else {
        /* wait for sometime to publish data */
        sleep(5);
    }


    /***********************************/
    /****** Publish device events ******/
    /***********************************/
    /* Set device event data */
    char *devdata = "{\"d\" : {\"TempID\": \"Test\", \"Reading\": 7 }}";
    rc = IoTPGateway_sendDeviceEvent(gateway, "testDeviceType", "testDevice", "deviceStatus", data, "json", QoS0, NULL);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf("Failed to publish device event. rc=%d\n", rc);
    } else {
        /* wait for sometime to publish data */
        sleep(5);
    }


    /*********************/
    /****** Cleanup ******/
    /*********************/
    /* Disconnect, and cleaup */
    rc = IoTPGateway_disconnect(gateway);
    if ( rc == IOTPRC_SUCCESS ) {
        /* Destroy client */
        rc = IoTPGateway_destroy(gateway);
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


## Gateway SDK Reference

For gateway configuration APIs, refer to <a href="https://ibm-watson-iot.github.io/iot-c/reference/iotp__config_8h.html" target="_iotsdkrefernce">Configuration Refernce Documentation</a>

For gateway client APIs, refer to <a href="https://ibm-watson-iot.github.io/iot-c/reference/iotp__gateway_8h.html" target="_iotsdkrefernce">Gateway Client Refernce Documentation</a>

For Return and error codes by C Client SDK, refer to <a href="https://ibm-watson-iot.github.io/iot-c/reference/iotp__rc_8h.html" target="_iotsdkrefernce">Return Code Refernce Documentation</a>


