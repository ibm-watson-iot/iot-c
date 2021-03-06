# Application Configuration

Application configuration can be broken down into required and optional configuration:

## Required Configuration
- `identity.orgId` Your organization ID.
- `identity.appId` A unique ID to identify the application.
- `auth.key` API Key to securely connect your application to Watson IoT Platform.
- `auth.token` API Token to securely connect your application to Watson IoT Platform.

## Optional Configuration
- `options.domain` A boolean value indicating which Watson IoT Platform domain to connect to (e.g. if you have a dedicated platform instance). Defaults to `internetofthings.ibmcloud.com`
- `options.logLevel` Controls the level of logging in the client, can be set to `error`, `warning`, `info`, or `debug`.  Defaults to `info`.
- `options.mqtt.port` A integer value defining the MQTT port.  Defaults to `8883`.
- `options.mqtt.transport` The transport to use for MQTT connectivity - `tcp` or `websockets`.
- `options.mqtt.cleanStart` A boolean value indicating whether to discard any previous state when reconnecting to the service.  Defaults to `False`.
- `options.mqtt.sessionExpiry` When cleanStart is disabled, defines the maximum age of the previous session (in seconds).  Defaults to `False`.
- `options.mqtt.keepAlive` Control the frequency of MQTT keep alive packets (in seconds).  Details to `60`.
- `options.mqtt.caFile` A String value indicating the path to a CA file (in pem format) to use in verifying the server certificate.  Defaults to `messaging.pem` inside this module.


The config parameter when creating a application client handle `IoTPApplication` expects to be passed as `IoTPConfig` object.
The application client SDK prrovides three APIs to create or update `IoTPConfig` object:

- `IoTPConfig_create` to read config parameters from an YAML file.
- `IoTPConfig_readEnvironment` to read config parameters from environment valiarbles.
- `IoTPConfig_setProperty` to set a config parameter by passing parameter name and value.

## YAML File Support

The client library allows one to easily pass in application device configuration using a file.

```
#include <iotp_application.h>

IOTPRC rc = IOTP_SUCCESS;
IoTPConfig *config = NULL;
IoTPApplication *application = NULL;
IoTPConfig_create(&config, "application.yaml");
IoTPApplication_create(&application, config);
```

### Minimal Required Configuration File

```yaml
identity:
    orgId: org1id
    appId: myApp
auth:
    key: a-org1id-Ab$76s)asj
    token: Ab$76s)asj8_s5
```

### Complete Configuration File

This file defines all optional configuration parameters.

```yaml
identity:
    orgId: org1id
    appId: myApp
auth:
    key: a-org1id-Ab$76s)asj
    token: Ab$76s)asj8_s5
options:
    domain: internetofthings.ibmcloud.com
    logLevel: debug
    mqtt:
        port: 8883
        transport: tcp
        cleanStart: true
        sessionExpiry: 7200
        keepAlive: 120
        caFile: /path/to/certificateAuthorityFile.pem
```


## Environment Variable Support

The client library allows one to easily pass in device configuration from environment variables.

```
#include <iotp_application.h>

IOTPRC rc = IOTP_SUCCESS;
IoTPConfig *config = NULL;
IoTPApplication *application = NULL;
IoTPConfig_create(&config, NULL);
IoTPConfig_readEnvironment(config);
IoTPApplication_create(&application, config);
```

### Minimal Required Environment Variables
- `WIOTP_IDENTITY_ORGID`
- `WIOTP_IDENTITY_APPID`
- `WIOTP_AUTH_KEY`
- `WIOTP_AUTH_TOKEN`

### Optional Additional Environment Variables
- `WIOTP_OPTIONS_DOMAIN`
- `WIOTP_OPTIONS_LOGLEVEL`
- `WIOTP_OPTIONS_MQTT_PORT`
- `WIOTP_OPTIONS_MQTT_TRANSPORT`
- `WIOTP_OPTIONS_MQTT_CAFILE`
- `WIOTP_OPTIONS_MQTT_CLEANSTART`
- `WIOTP_OPTIONS_MQTT_SESSIONEXPIRY`
- `WIOTP_OPTIONS_MQTT_KEEPALIVE`


## Use API to set a specific config parameter

The client library allows one to set config parameter using `IoTPConfig_setProperty` API.

```
#include <iotp_application.h>

IOTPRC rc = IOTP_SUCCESS;
IoTPConfig *config = NULL;
IoTPApplication *application = NULL;
IoTPConfig_create(&config, NULL);
IoTPConfig_setProperty(config, "identity.orgId", "a52xin");
IoTPConfig_setProperty(config, "identity.appId", "rasberry-pi");
IoTPConfig_setProperty(config, "auth.key", "a-a52xin-Ab$76s)asj");
IoTPConfig_setProperty(config, "auth.token", "Ab$76s)asj8_s5");
IoTPApplication_create(&application, config);
```

