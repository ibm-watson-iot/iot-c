# Application SDK

The application SDK includes a shared library that exposes functions to build application client that 
runs on IoT applications.

- `libwiotp-sdk-application-as.so`

Support APIs for creating a application client and managing connection to the Watson IoT Platform service:

- `IoTPConfig_create()`
- `IoTPApplication_create()`
- `IoTPApplication_connect()`

Support APIs for working with events and commands: 

- `IoTPApplication_sendEvent()`
- `IoTPApplication_setEventCallback()`
- `IoTPApplication_setCommand()`
- `IoTPApplication_setCommandsHandler()`
- `IoTPApplication_subscribeToCommands()`
- `IoTPApplication_sendDeviceEvent()`
- `IoTPApplication_subscribeToDeviceCommands()`

## Configuration

Application configuration is passed to the client via the `IoTPConfig` object when you create 
the client instance or handle `IoTPApplication`. See the [configure applications](config.md) section 
for full details of all available options to configure a client instance.

The config handle `IoTPConfig` created using `IoTPConfig_create()` must be cleared 
using `IoTPConfig_clear()` to avoid handle leak. The application handle `IoTPApplication` created 
using `IoTPApplication_create()` must be destroyed using `IoTPApplication_destory()` to avoid handle leak.
i

## Connectivity

`IoTPApplication_connect()` & `IoTPApplication_disconnect()` APIs are used to manage the MQTT connection
to the Watson IoT Platform service that allows the application to handle commands and publish events.


## Publishing Events

Application can publish data collected by the application or on behalf of other devices. 
Events can be published with any of the three quality of service (QoS) levels that are defined
by the MQTT protocol. By default, events are published with a QoS level of 0.
Optionally you can send MQTT V5 properties. For details on MQTT V5 properties, refer to 
<a href="https://www.eclipse.org/paho/files/mqttdoc/MQTTAsync/html/struct_m_q_t_t_properties.html" target="_blank">MQTTProperty</a>.

To send events, you can use `IoTPApplication_sendEvent()` API. This API takes upto 8 arguments:

- `application` Client handle
- `typeId` Device type Id
- `deviceId` Device Id
- `event` Name of the event
- `data` Data for this event
- `msgFormat` Format of the data for this event
- `qos` MQTT quality of service level to use (`0`, `1`, or `2`)
- `props` MQTT V5 properties

You can set a callback to get send event response using `IoTPApplication_setEventCallback()` API. 
The callback returns success and failure responses. The success respose has different implications depending
on the level of qos used to publish the event:

- qos 0: the client has asynchronously begun to send the event
- qos 1 and 2: the client has confirmation of delivery from the platform


## Publishing Commands

Application can publish command for a device or gateway. To publish commands, 
you can use `IoTPApplication_sendCommand()` API. This API takes upto 8 arguments:

- `application` Client handle
- `typeId` Device type Id
- `deviceId` Device Id
- `command` Name of the command
- `data` Data for this command
- `msgFormat` Format of the data for this command
- `qos` MQTT quality of service level to use (`0`, `1`, or `2`)
- `props` MQTT V5 properties


## Handling Events

An application client can susbcribe to device or gateway events using `IoTPApplication_subscribeToEvents()` API.
To process specific commands, you need to register a event callback method, using `IoTPApplication_setEventHandler()`.

The messages are returned as an instance of the `Event` class with the following attributes:

- `type`: Client type ID
- `id`: Client ID
- `event`: Identifies the event
- `format`: Format that the command was encoded in, for example `json`
- `payload`: Data for the payload
- `payloadlen`: Size of the payload buffer


## Handling Commands

An application client can susbcribe to a gateway or device command using `IoTPApplication_subscribeToCommands()` API.
To process specific commands, you need to register a command callback method, using `IoTPApplication_setCommandHandler()`.

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
    applicationId: testApplication
auth:
    key: a-org1id-htan65Wisp
    token: hP!ax98sjcD
```

### Sample code

```
#include <iotp_application.h>

/* Application command callback function. Application developers can customize this function for their use case. */
void  applicationCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received application command. Type=%s ID=%s CommandName=%s Format=%s Len=%d\n",
        type?type:"", id?id:"", commandName?commandName:"", format?format:"", (int)payloadSize);
    /* Application developers - add your custom code to process application command and its payload */
}

/* Application event callback function. */
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
    IoTPApplication *application = NULL;

    /*******************************************************/
    /****** Create application client instance and connect ******/
    /*******************************************************/
    /* Create IoTPConfig object, use it to create IoTPApplication handle, and connect client */
    rc = IoTPConfig_create(&config, "configFile.yaml");
    if ( rc == IOTPRC_SUCCESS ) {
        rc = IoTPApplication_create(&application, config);
        if ( rc == IOTPRC_SUCCESS ) {
            rc = IoTPApplication_connect(application);
        }
    }
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "Failed to create application client: rc=%d reason=%s\n", rc, IOTPRC_toString(rc));
        exit(1);
    }


    /******************************/
    /****** Process commands ******/
    /******************************/
    /* Set application command callback for all commands and subscribe to all commands in any supported format */
    rc = IoTPApplication_setCommandHandler(application, applicationCommandCallback, "+", "+", "+", "+");
    if ( rc == IOTPRC_SUCCESS ) 
        rc = IoTPApplication_subscribeToCommands(application, "+", "+", "+");
    }
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "Failed to set command handler or subscribe to device commands. rc=%d reason:%s\n", rc, IOTPRC_toString(rc));
    }


    /***********************************/
    /****** Publish device events ******/
    /***********************************/
    /* Set event callback to process event response and send event */
    rc = IoTPApplication_setEventCallback(application, eventCallback);
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
    rc = IoTPApplication_sendEvent(application, "testDevType", "testDev", "status", data, "json", QoS0, properties);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf("Failed to publish device event. rc=%d\n", rc);
    } else {
        /* wait for sometime to publish event */
        sleep(5);
    }


    /*********************/
    /****** Cleanup ******/
    /*********************/
    /* Disconnect, and cleaup */
    rc = IoTPApplication_disconnect(application);
    if ( rc == IOTPRC_SUCCESS ) {
        /* Destroy client */
        rc = IoTPApplication_destroy(application);
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


## Application SDK Reference

For application configuration APIs, refer to <a href="https://https://ibm-watson-iot.github.io/iot-c/reference/iotp_config_8h.html" target="_iotsdkrefernce">Configuration Refernce Documentation</a>

For application client APIs, refer to <a href="https://https://ibm-watson-iot.github.io/iot-c/reference/iotp_application_8h.html" target="_iotsdkrefernce">Application Client Refernce Documentation</a>

For Return and error codes by C Client SDK, refer to <a href="https://https://ibm-watson-iot.github.io/iot-c/reference/iotp_rc_8h.html" target="_iotsdkrefernce">Return Code Refernce Documentation</a>


