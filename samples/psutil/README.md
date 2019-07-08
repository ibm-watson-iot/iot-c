# IBM Watson IoT Platform cpsutil Device Client

Device code for sending system utilization data to IBM Watson IoT Platform.

The following data points are collected:
 * CPU utilization (%)
 * Total system memory (KB)
 * Memory used by psutil (KB)
 * Outbound network utilization across all network interfaces (KB/s)
 * Inbound network utilization across all network interfaces (KB/s)
 * Disk read rate (KB/s)
 * Disk write rate (KB/s)


## Event Format

- `name` hostname of the system
- `cpu` percent of CPU used by this process
- `mem` percent of memory used by this process
- `network.up` Outbound network utilization (KB/s)
- `network.down` Inbount network utilization (KB/s)
- `disk.write` disk write rate (KB/s)
- `disk.read` disk read rate (KB/s)

## Before you Begin

Register a device with IBM Watson IoT Platform.  

For information on how to register devices, see the [Connecting Devices](https://www.ibm.com/support/knowledgecenter/SSQP8H/iot/platform/iotplatform_task.html) topic in the IBM Watson IoT Platform documentation.  

At the end of the registration process, make a note of the following parameters: 
   - Organization ID
   - Device Type
   - Device ID
   - Authentication Token  

## Docker

The easiest way to test out the sample is via the [wiotp/psutil](https://cloud.docker.com/u/wiotp/repository/docker/wiotp/psutil) Docker image provided and the `--quickstart` command line option.

```
$ docker run -d --name cpsutil wiotp/cpsutil --quickstart
cpsutil
$ docker logs -tf cpsutil
2019-05-07T11:09:19.672513500Z 2019-05-07 11:09:19,671   wiotp.sdk.device.client.DeviceClient  INFO    Connected successfully: d:quickstart:iotcpsutil:242ac110002
```

To connect as a registered device in your organization you must set the following environment variables in the container's environment. These variables correspond to the device parameters for your registered device: 
- `WIOTP_IDENTITY_ORGID`
- `WIOTP_IDENTITY_TYPEID`
- `WIOTP_IDENTITY_DEVICEID`
- `WIOTP_AUTH_TOKEN`.

The following example shows how to set the environment variables:

```
$ export WIOTP_IDENTITY_ORGID=myorgid
$ export WIOTP_IDENTITY_TYPEID=mytypeid
$ export WIOTP_IDENTITY_DEVICEID=mydeviceid
$ export WIOTP_AUTH_TOKEN=myauthtoken
$ docker run -d -e WIOTP_IDENTITY_ORGID -e WIOTP_IDENTITY_ORGID -e WIOTP_AUTH_TOKEN --name psutil wiotp/psutil
psutil
$ docker logs -tf psutil
2019-05-07T11:09:19.672513500Z 2019-05-07 11:09:19,671   wiotp.sdk.device.client.DeviceClient  INFO    Connected successfully: d:myorgid:mytypeid:mydeviceid
```


