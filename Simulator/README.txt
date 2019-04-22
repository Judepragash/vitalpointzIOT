This repository contains node-RED flows that simulate IOT device to vitalpointz secure IOT Core/Platform. 
These flows are provided to illustrate the device on-boarding API, way of publishing/subscribing messages into 
vitalpointz secure IOT Core/Platfrom. Import these flows in to node-RED installed on your laptop or VM and follow the 
inline documentation. 

Pre-Reqs: These flows would requrie you to install these nodes using 'manage palette' menu in your Node-RED instance
(a)node-red-contrib-string (b)node-red-contrib-msg-speed


File: IOT_Device_Manual_TLS
When a IOT Device is added under vitalpointz secure IOT Core/Platform, authcode is returned. This flow demonstrartes 
step-by step how a device can onboard itself using the HTTPS REST APIs and authcode. It also demonstrates sending message to vitalpointz
secure IOT Core/Platform using MQTT and HTTPS REST. 

File:IOT_Device_Manual_Plaintext
Same as 'IOT_Device_Manual_TLS', but using unsecure HTTP-REST. 

File:IOT_Device_Auto_TLS
Vitalpointz secure IOT Core/platform allows the user to add devices in bulk. When the devices are added in this way,
loginkey is provided, which has two components. User id and Tenant Id. This flow explains how to on-board bulk of
devices using the above ids. It also shows how to send messages using MQTT and HTTPS REST.

File:IOT_Device_Auto_Plaintext
Same as 'IOT_Device_Auto_TLS' but with HTTP REST.
