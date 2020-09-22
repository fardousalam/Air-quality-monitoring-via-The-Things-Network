# Air quality monitoring via The Things Network(TTN)

This project is conducted in Hochschule Ravensburg Weingarten in Summer 2018 as a study requirements of Embedded Computing Project. The goal of this project is to monitor the air quality continously.

LoRA WAN stands for Long Range Wide Area Network. LoRa enables long-range transmissions (more than 10 km in rural areas) with low power consumption. LoRa uses license-free sub-gigahertz radio frequency bands like 433 MHz, 868 MHz (Europe), 915 MHz (Australia and North America) and 923 MHz (Asia). Due to its low power, long range transmission capability(generally more than 1okm in rural areas) it is used in different types of industries to connect IoT (Internet of Things) devices.
The Things Network (TTN) is a community-based initiative to establish an energy-saving wide-area network for the IoT devices. It has covered a large area by LoRAWAN Gateway. So, the core concept is that there is a LoRAWAN gateway and with that one gateway one or more than one node/application can communicate. 

Atfirst a LoRa WAN Gateway needs to be installed. The hardware settings for LoRAWAN gateway and node can be found on this link. 
https://www.instructables.com/id/Use-Lora-Shield-and-RPi-to-Build-a-LoRaWAN-Gateway

In this project the air quality of the environment is tested and the data are sent to the TTN. A dust sensor is used to get the corrosponding data of air particle and the datas are proceesed in arduino and finally it is sent to the TTN. As long as the all hardware are functioning the data are sent and updated continously. 


