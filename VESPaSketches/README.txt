## Vitalpointz VESPa Agent
  This library provides a set of functions to interact with Vitalpointz IoT platform (VESP)

## Examples
  The library comes with an follwing example sketches for reference.
  See File > Examples > VESPa > 
    example 1  - vespa_config_eeprom_write (Writes vespa configuration to eeprom at starting addres 0)
    example 2  - nodemcu_vespa_ota (Reads vespa config from eeprom and performs vesp onboarding procedure)

## Limitations
  The sketch and VESPa apis are tested on esp8266 platform only.

## Compatible Hardware
  The library uses about 273kb storage and 34kb DRAM for basic function and wifi for coonectivity.
  The micro-controller must incorporate the basic hardware requirements. Follows the list of hardware
  its tested on.
   - ESP8266
	- NodeMCUv2
	- NodeMCUv3
	- ESP01

## Dependencies
  - Board package
    - ESP8266 Board Package for Arduino IDE
	Add following URL to preference->additional boards
        http://arduino.esp8266.com/stable/package_esp8266com_index.json
        Goto board manager search esp8266 and install board packages
  - Libraries
    - ESP8266Wifi
    - ESP8266HTTPClient
    - PubSubClient
    - ESP8266httpUpdate

    NOTE: Need to increase MQTT_MAX_PACKET_SIZE in Arduino/libraries/PubSubClient/src/PubSubClient.h 
          to send bigger MQTT Messages, default configured size is 128.

  All these libraries can be installed from Library Manager Sketch > Include Library > Manage Libraries

## Getting Started
 - Ensure NodeMCU is detected on Arduino IDE
    Check Tools > Port and look for newly detected port
    Test by installing basic sketch blink to verify if led is blinking.

 - Install VESPa Library 
   Sketch> Include Library> Add .zip library and select downloaded VESPa<veraion>.zip file.

 - Running VESPa MC 
   - You can now choose example "vespa_config_eeprom_write.ino" 
     configure vespa with needed configuration and write to eeprom.
   - After configuration is complete choose example "nodemcu_vespa_ota.ino" 
     for VESP onboarding and connecting to VESP cloud.
   - Defult baudrate for serial monitor is 115200.
   - For OTA Please create a distribution via vesp UI under OTA S/W Update tab.
     Create an working uploadable image via IDE sketch>"export compiled binary" option. It will generate a .bin file in <Installed path>/Arduino/library/VESPa/examples/<sketch name>/ dir.
     NOTE: Device need to have twice the flash space than used by single image.

## License
 - Please contact vitalpointz for howto videos, more details and help.

## Known Issues
 - After the vesp updates the device image via OTA, Flashing the new image via IDE may causing eeprom read failure on init data in random scenarios. For a work around: Please flash the image again.

