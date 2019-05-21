# Basic Concepts

## Thing Classes

The Internet is made up of "things", the most important concept to get to terms with when working with Watson IoT Platform is the idea of applications, devices, & gateways as three distinct classes of "thing" in your Internet of Things solution.  

Getting your **physical device model** right is essential to building a solution that will allow you to take advantage of all the advanced capabilities of Watson IoT.


### Devices

Devices are things that send data into the service (directly, or indirectly), and respond to commands directed at them.

- Send **events**
- Recieve **commands**

!!! tip
    Devices in Watson IoT Platform are intended to mirror the physical deployment of hardware that will generate IoT data, regardless of whether it directly connects to the internet.

!!! warning
    If you deploy 6 pieces of hardware each with seperate firmware, software, etc avoid the temptation to think that tracking these as individual devices has no value.  Merging them into an "abstract device" representing all 6 when you register your physical device model in Watson IoT will make it more difficult to use advanced features of the platform as you explore Watson IoT Platform's advanced capabilities for device and data management.

!!! tip
    Device can collect large amount of data. Depending on the use case, users will have to predict which data provides the most value. They can predict this based on impact of data collection, power consumption of the device, and data transmission speed and latency, how the data is consumed by back-end applications. Users will also have to decide on how to represent the event from devices.  Sometimes it is difficult to decide what constitutes a device. For example, a commercial refrigeration units (used in departmental stores) may have multiple bins or compartments with temperature sensors in each of the compartments. In this case, each compartment can be modeled as a device with its own unique device ID or the whole unit could be one device with temperature data from each compartment.  So, events could be modeled as different levels of abstraction. Example events:

```
<b>Example 1:</b> Modeled as each temperature sensor as a device:

{"deviceId":"refUnit1_compartment1","temp":18}
{"deviceId":"refunit1_compartment2","temp":20}


<b>Example 2:</b> Modeled as refrigeration unit as a device:

{"deviceId":"refunit1","compartment1_temp":18, "compartment2_temp":20}


<b>Example 3:</b> Modeled as refrigeration unit as a device and temp data in an array:

{"deviceId":"refunit1","compartments":[{"temp":18},{"temp":20}]}


Considering the size of event, and consumbility by back-end application,
Example 3 is a better option to use compared with Example 1 and Example 2.
```

### Gateways

Gateways are things that send data into the service, respond to commands, are able to send data from other devices, and relay commands to other devices.

- Send **events**
- Recieve **commands**
- Send **events** on behalf of other devices
- Recieve **commands** sent to other devices

!!! tip
    Use gateways when you are developing a solution where multiple physical devices exist that will not each directly communicate with Watson IoT Platform, but instead will report to a local device, which serves as a central contact point to the service.  

    
    - Each physical device should be registered to the platform as a device, even though it will not connect directly.
    - The central point of contact is your gateway.
    - The gateway should send multiple events on behalf of the local devices, rather than claiming ownership of the data by submitted the events as if the data came from the gateway itself.


### Applications

Applications are the most powerful class of thing in Watson IoT Platform.

- Send **events** on behalf of devices
- Send **commands** to devices
- Recieve **commands** sent to devices
- Work with the IBM Watson IoT Platform APIs

!!! tip
    Applications are able to function as a gateway into the service, but should only be used as such when you view the gateway as an abstract entity in your solution rather than something physical to be managed on-site.  If you assoicate the central point of contact with the platform as a specific piece of hardware it should be implemented as a gateway.

!!! warning
    Applications capabilities vary wildly depending on the permissions granted to the application by the API key that it uses to connect.  It is important to align the role granted to the API key used by the application to the capabilities of the application.


