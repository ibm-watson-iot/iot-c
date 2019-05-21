# Return and Error Codes

The return and error codes in the SDK are common across all libraries (device, gateway, application, managed device, and managed gateway). Below is a summary of the returned return and error codes used in this SDK.

## Codes returned by C SDK

- Retrun code 0 indicates a success reponse
- Return code 1 indicates a generic failure response
- Return codes 1001 to 1500 indicates failure response

## Codes returned by dependent Paho MQTT C library

- Return code less than 0 are specific error codes returned by Paho MQTT C Client library. For details, refer to [MQTTReasonCodes](https://www.eclipse.org/paho/files/mqttdoc/MQTTAsync/html/_m_q_t_t_reason_codes_8h.html){target="_blank"} in Paho Asynchronous MQTT C Client Library.


