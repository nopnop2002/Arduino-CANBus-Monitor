# Arduino-CANBus-Monitor
MCP2515 canbus module to Monitor Canbus traffic.

![USBCAN-001](https://user-images.githubusercontent.com/6020549/86521937-b7b74c80-be91-11ea-90c4-9585b1489965.jpg)

# Background

Windows application for monitoring CANBus is published [here](https://github.com/SeeedDocument/USB-CAN_Analyzer/tree/master/res/USB-CAN%20software%20and%20drive(v7.10)/Program).   
This tool is designed to be used with this hardware.   
![bazaar487719_1](https://user-images.githubusercontent.com/6020549/86521939-b9811000-be91-11ea-8dfc-2bc8e24d7f0c.jpg)

So, I made the hardware side with Arduino UNO+MCP2515 canbus module.   


# Application structure
```
+-------------+           +-------------+           +-------------+
| Windows     |           |             |           |   MCP2515   |
| Application |<--(USB)-->| Arduino-UNO |<--(SPI)-->|   canbus    |<--(CAN BUS)-->CAN Network
|             |           |             |           |   module    |
+-------------+           +-------------+           +-------------|
```

# Hardware requirements

- Windows PC   
Windows applications can be downloaded [here](https://github.com/SeeedDocument/USB-CAN_Analyzer/tree/master/res/USB-CAN%20software%20and%20drive(v7.10)/Program).

- Arduino UNO   
It also works on ATMega2560.   

- MPC2515 canbus module   
You can get it from AliExpress or eBay for under $2.   
![MCP2515_CAN_MODULE](https://user-images.githubusercontent.com/6020549/86521994-7ffcd480-be92-11ea-9248-8a3f06a0ae2f.JPG)


# Dependent libraries

- MCP_CAN lib   
You can download from [here](https://github.com/coryjfowler/MCP_CAN_lib).


# Connection with CAN bus module

|Module||UNO|
|:-:|:-:|:-:|
|INT|--|Pin#2|
|SCK|--|Pin#13|
|SI|--|Pin#11|
|SO|--|Pin#12|
|CS|--|Pin#10|
|GND|--|GND|
|VCC|--|5V|


# How to use

- Write Arduino Code on UNO.   
- Start a Windows application.   

# User manual
See here.   

# Trouble shooting
UNO hardware serial is used for USB communication.   
The Arduino-IDE serial monitor cannot be used.   
UNO debugging uses software serial.   
```
SoftwareSerial mySerial(4, 5); // RX, TX
```


