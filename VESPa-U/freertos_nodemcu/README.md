[![N|Solid](https://user-images.githubusercontent.com/37981409/60482555-a2707680-9caf-11e9-8ee0-c128a9585d06.png)](https://vitalpointz.io/)

Code and steps in this project meant to work with Vitalpointz IoT Core/Core Lite platform. If you are not familier with Vitalpointz IoT Platform and offerings, refer to the links given at the end.

# VESPa-U for NodeMCU on FreeRTOS & TLS
### Steps to Setup the Environment
##### Supported IoT Board
 - NodeMCU
##### Tools, SDKs and Drivers used in this guide tested on
 - OSX, preinstalled with Python, Git, NodeMCU serial driver (CP2102), XCode Command line Tools  
##### Follow the links below to setup SDK and toolchain for ESP platform in OSX
* [Follow this link](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/macos-setup.html) to setup standard toolchain for esp8266
* [Follow this link](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html#get-esp8266-rtos-sdk) to setup esp8266 FreeRTOS SDK

##### Set path & environment variables for the SDK and toolchain to work
Write and source below lines in “~/.profile” file to declare environment variables. Source the profile everytime you build. 
```sh
export PATH=$PATH:$HOME/esp/xtensa-lx106-elf/bin
export IDF_PATH=$HOME/esp/ESP8266_RTOS_SDK
```
### Steps for Configuration & Flashing
#### Initial Configuration
  - Download and unzip VESPa project file into the working directory 
  - Run “make menuconfig” inside the vespa directory (vespa_rtos_secure)
  - Update the “WiFi SSID” & “Password” under “Example Configuration”.
![N|Solid](https://user-images.githubusercontent.com/37981409/60417235-9330f080-9bfd-11e9-9d9d-390bafbc583a.png)
  - Update the “Serial Port” config under “Serial flasher config” (optional)
![N|Solid](https://user-images.githubusercontent.com/37981409/60417246-975d0e00-9bfd-11e9-8414-9ef547f021bb.png)
  - Save & Exit
#### Configuration, Flashing & Onboarding
  - Open <working dir>/vespa_rtos_secure/main/vespa.c in a text editor
  - Update line 17, 18, 19 & 20 with “Vitalpointz IoT Platform URL”, “Username”, “password” & “auth code”. [Where to get these?](https://judepragash.github.io/vitalpointzIOT_Docs/ProdMan/ProdManPgs/dev_registration.html) 
  - Save the file
  - Flash the NodeMCU with command “make flash”
```sh
………output truncated!!
LD build/vespa-rtos-secure.elf
esptool.py v2.4.0
Flashing binaries to serial port /dev/cu.SLAB_USBtoUART (app at offset 0x10000 )...
esptool.py v2.4.0
Connecting........_
Chip is ESP8266EX
Features: WiFi
MAC: 84:0d:8e:86:6b:78
Uploading stub...
Running stub...
Stub running...
Configuring flash size...
Compressed 10816 bytes to 7169...
Wrote 10816 bytes (7169 compressed) at 0x00000000 in 0.6 seconds (effective 135.8 kbit/s)...
Hash of data verified.
Compressed 431648 bytes to 284022...
Wrote 431648 bytes (284022 compressed) at 0x00010000 in 25.1 seconds (effective 137.5 kbit/s)...
Hash of data verified.
Compressed 3072 bytes to 83...
Wrote 3072 bytes (83 compressed) at 0x00008000 in 0.0 seconds (effective 2046.5 kbit/s)...
Hash of data verified.
```
  - Once compilation & flashing done, run “make monitor” to see the serial console of the NodeMCU
```sh
………output truncated!!
I (6795) VESPA: vespa onboarding completed
vespa Task finish.
MQTT uri         = 'mqtts://mqtt-5d1596eb276754003747b6a4.dedemeetup.vitalpointz.com:10001'
I (6815) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE
Calling mqtt_start 
I (18805) MQTT_CLIENT: Sending MQTT CONNECT message, type: 1, id: 0000
I (18835) VESPA: MQTT_EVENT_CONNECTED
Sending mqtt message = 'msgno=1,temprature_celcius=21,wind_mps=2,pressure_hpa=1111'
Sending mqtt message = 'msgno=2,temprature_celcius=21,wind_mps=2,pressure_hpa=1111'
```
  - Alternatively, both sub-commands can be issued together to see serial console right after flashing. i.e. “make flash monitor”
#### Please Note
> The default sdkconfig is packaged to take care 
> of prerequisites of freertos configuration for 
> VESPa like main app or task stack size etc. 
> SSL library used is mbedtls.

> This is reference implementation of how to use/port 
> VESPa APIs on a RTOS. This implementation is nodemcu specific.

> To know more on APIs Please read API document. 

### Important Links
* [Official Vitalpointz Site](https://vitalpointz.io/)
* [Vitalpointz IoT Core Lite](https://marketplace.digitalocean.com/apps/vitalpointz-iot-core-lite)
* [Vitalpointz IoT Product Docs ](https://judepragash.github.io/vitalpointzIOT_Docs/ProdDoc.html)
