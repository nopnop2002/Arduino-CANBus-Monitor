/*
 * USB CAN for Arduino
 */
#include <SoftwareSerial.h>
#include <mcp_can.h>
#include <SPI.h>

SoftwareSerial mySerial(4, 5); // RX, TX

#define baudrate 115200
//#define baudrate 230400
//#define baudrate 460800
//#define baudrate 921600

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

unsigned long lastTime;

#define MAX_VALID 20
typedef struct {
  unsigned char cmd;
  int counter;
  unsigned long validId[MAX_VALID];
} CONFIG_t;

CONFIG_t config;

#define NOT_SUPPORT "This command does not support"

unsigned char calcCRC(unsigned char *buffer, int length) {
  unsigned char calc = 0;
  for (int i=0; i<length; i++) {
    //mySerial.print(buffer[i],HEX);
    //mySerial.print(" ");
    calc = calc + buffer[i];
  }
  //mySerial.println();
  return calc;
}

bool analyzeCommand(unsigned char c, unsigned char *buffer, int *buflen) {

  static int _status = 0;
  static unsigned char _cmd = 0;
  static int _len = 0;
  static int _last = 0;
  static unsigned char _crc = 0;
  static bool _save = false;
  
#if 0 
  mySerial.print("_len=");
  mySerial.print(_len);
  mySerial.print(" _status=");
  mySerial.print(_status);
  mySerial.print(" _cmd=");
  mySerial.print(_cmd,HEX);
  mySerial.print(" _last=");
  mySerial.print(_last);
  mySerial.print(" _crc=");
  mySerial.print(_crc);
  mySerial.println();
#endif
   
  if (_status == 0 && c == 0xAA) {
    buffer[0] = c;
    _len = 1;
    _status = 1;
    _cmd = 0;
    _save = true;
    return false;
  } else if (_status == 1) {
    _cmd = c;
    unsigned char _cmd0 = (c & 0x0F);
    unsigned char _cmd1 = (c & 0xF0) >> 4;

#if 0
    mySerial.print("_cmd=");
    mySerial.print(_cmd, HEX);
    mySerial.print(" _cmd0=");
    mySerial.print(_cmd0, HEX);
    mySerial.print(" _cmd1=");
    mySerial.print(_cmd1, HEX);
    mySerial.println();
#endif
    
    if (_cmd == 0x55) {
      _crc = 0;
      _status = 2;
    } else if (_cmd1 == 0xC || _cmd1 == 0xD) { // Send Standard frame
      _last = _cmd0 + 5; //13;
      _crc = 0x55;
      _status = 9;
    } else if (_cmd1 == 0xE || _cmd1 == 0xF) { // Send Extended frame
      _last = _cmd0 + 7; //15
      _crc = 0x55;
      _status = 9;
    }
    buffer[1] = c;
    _len++;
    return false;
  } else if (_status == 2) {
    if (c == 0x4) { // Monitor
      _last = 20;
      _status = 9;
    } else if (c == 0x6) { // Change bps
      _last = 20;
      _status = 9;
    } else if (c == 0x7) { // Transparent
      _last = 20;
      _status = 9;
    } else if (c == 0x10 || c == 0x11) { // Configure Receive ID
      _status = 3;
    } else if (c == 0x12) { // Set and Start(auto)
      _last = 20;
      _status = 9;
    } else if (c == 0x13) { // Set and Start(manual)
      _last = 20;
      _status = 9;
    } else if (c == 0x30) { // Downlaod send (V8.0)
      _status = 5;
    } else if (c == 0x33) { // Downlaod reply (V8.0)
      _status = 6;
    } else if (c == 0x32 || c == 0x34) { // Enable/Disable (V8.0)
      _status = 7;
    }
    buffer[_len] = c;
    _len++;
  } else if (_status == 3) {
    _last = c*4+5;
    buffer[_len] = c;
    _len++;
    _status = 9;
  } else if (_status == 5 || _status == 6) {
    _last = c*18+6;
    if (_status == 6) _last = c*20+6;
    buffer[_len] = c;
    _len++;
    _save = false; // not save 
    _status = 9;
  } else if (_status == 7) {
    _last = 20;
    buffer[_len] = c;
    _len++;
    _status = 9;
  } else {
    if (_save) buffer[_len] = c;
    _len++;
    //if (_cmd == 0x55 && _len == 20) {
    if (_len == _last) {
      _status = 0;
      if (_save == true) {
        if (_crc == 0) {
          _crc = calcCRC(&buffer[2], _len-3);
        }
        //mySerial.print("_crc=");
        //mySerial.println(_crc,HEX);
        if (_crc == c) {
          *buflen = _len;
          return true; 
        }
      } else {
        *buflen = 3;
        return true; 
      }
    }
  }
}

void Monitor(unsigned char recvError, unsigned char sendError, unsigned char *buffer, int buflen){
  mySerial.println("Monitor");
  buffer[0] = 0xAA;
  buffer[1] = 0x55;
  buffer[2] = 0x04;
  buffer[3] = recvError;
  buffer[4] = sendError;
  buffer[5] = 0; // Error Flag(0:Normal/1;Error pas)
  buffer[6] = 0; // Bus State(0:Bus-on/1:Bus-off)
  for(int i=7;i<19;i++) buffer[i] = 0x00;
  buffer[19] = calcCRC(&buffer[2], 17);
  for(int i=0;i<20;i++) {
    Serial.write(buffer[i]);
  }
}

/*
#define CAN_4K096BPS 0
#define CAN_5KBPS    1
#define CAN_10KBPS   2
#define CAN_20KBPS   3
#define CAN_31K25BPS 4
#define CAN_33K3BPS  5
#define CAN_40KBPS   6
#define CAN_50KBPS   7
#define CAN_80KBPS   8
#define CAN_100KBPS  9
#define CAN_125KBPS  10
#define CAN_200KBPS  11
#define CAN_250KBPS  12
#define CAN_500KBPS  13
#define CAN_1000KBPS 14
*/
byte setBitRate(unsigned char speed) {
  //mySerial.print("CAN bps=");
  //mySerial.println(speed);
  switch(speed) {
    case 1:
      return CAN_1000KBPS;
    case 2:
      return -1;
    case 3:
      return CAN_500KBPS;
    case 4:
      return -1;
    case 5:
      return CAN_250KBPS;
    case 6:
      return CAN_200KBPS;
    case 7:
      return CAN_125KBPS;
    case 8:
      return CAN_100KBPS;
    case 9:
      return CAN_50KBPS;
    case 10:
      return CAN_20KBPS;
    case 11:
      return CAN_10KBPS;
    case 12:
      return CAN_5KBPS;
  }
}

void Set_and_Start(unsigned char *buffer, int buflen){
  mySerial.print("Set_and_Start BitRate=");
  mySerial.print(buffer[3], HEX);
  mySerial.print(" FrameType=");
  mySerial.print(buffer[4], HEX);
  mySerial.print(" FilterID=");
  mySerial.print(buffer[8],HEX);
  mySerial.print("-");
  mySerial.print(buffer[7],HEX);
  mySerial.print("-");
  mySerial.print(buffer[6],HEX);
  mySerial.print("-");
  mySerial.print(buffer[5],HEX);
  mySerial.print(" MaskD=");
  mySerial.print(buffer[12],HEX);
  mySerial.print("-");
  mySerial.print(buffer[11],HEX);
  mySerial.print("-");
  mySerial.print(buffer[10],HEX);
  mySerial.print("-");
  mySerial.print(buffer[9],HEX);
  mySerial.println();

  
  byte speed = setBitRate(buffer[3]);
  if (speed < 0) {
    mySerial.println("This bit rate is not supported");
    return;
  }
  mySerial.print("speed=");
  mySerial.println(speed);
  
  //if(CAN0.begin(MCP_ANY, speed, MCP_8MHZ) == CAN_OK) {
  if(CAN0.begin(MCP_STDEXT, speed, MCP_8MHZ) == CAN_OK) {
    mySerial.println("MCP2515 Initialized Successfully!");
  } else {
    mySerial.println("Error Initializing MCP2515...");
    while(1);
  }
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                     // Configuring pin for /INT input

  unsigned long mask;
  unsigned long filter;

  if (buffer[4] == 1) { // Standard Frame
    mySerial.println("Standard Frame");
    mask = (buffer[10] & 0x7);
    mask = mask << 8;
    mask = mask + buffer[9];
    mask = mask << 16;
    filter = (buffer[6] & 0x7) << 8;
    filter = filter + buffer[5];
    filter = filter << 16;
  } else { // Extended Frame
    mySerial.println("Extended Frame");
    mask = (buffer[12] & 0x1F);
    mask = mask << 24;
    mask = mask + (buffer[11] << 16);
    mask = mask + (buffer[10] << 8);
    mask = mask + buffer[9];

    filter = (buffer[8] & 0x1F);
    filter = filter << 24;
    filter = filter + (buffer[7] << 16);
    filter = filter + (buffer[6] << 8);
    filter = filter + buffer[5];
  }
  mySerial.print("mask=0x");
  mySerial.print(mask,HEX);
  mySerial.print(" filter=0x");
  mySerial.print(filter,HEX);
  mySerial.println();
  CAN0.init_Mask(0,0,mask);                // Init first mask...
  CAN0.init_Filt(0,0,filter);              // Init first filter...
  CAN0.init_Mask(1,0,mask);                // Init second mask... 
  CAN0.init_Filt(1,0,filter);              // Init third filter...
    
  mySerial.println("Set_and_Start end");
  
}

void Configure_Receive_ID(unsigned char *buffer, int buflen){
  mySerial.print("Configure_Receive_ID counter=");
  mySerial.println(config.counter);
  config.cmd = buffer[2];
  config.counter = buffer[3];
  unsigned long id;
  unsigned long _id;
  for(int i=0;i<buffer[3];i++) {
    _id = (buffer[i*4+7] & 0x1F);
    id = _id << 24;
    _id = buffer[i*4+6];
    id = id + (_id << 16);
    id = id + (buffer[i*4+5] << 8);
    id = id + buffer[i*4+4];
    //mySerial.print("id=0x");
    //mySerial.println(id,HEX);
    if (i < MAX_VALID) config.validId[i] = id;
  }

  for(int i=0;i<config.counter;i++) {
    if (config.cmd == 0x10) mySerial.print("ValidId=0x");
    if (config.cmd == 0x11) mySerial.print("InvalidId=0x");
    mySerial.println(config.validId[i], HEX);
  }
}
  
byte Send_Standard_frame(unsigned char frameType, byte frameLen, unsigned char *frameId, unsigned char *frameData){
  mySerial.print("Send_Standard_frame frameType=");
  mySerial.print(frameType, HEX);
  mySerial.print(" frameLength=");
  mySerial.println(frameLen);
  
  unsigned long stdID = (frameId[1] & 0x7);
  stdID = stdID << 8;
  stdID = stdID + frameId[0];
  if (frameType == 0xD) stdID = stdID | 0x40000000; // Remote Frame

  byte sndStat = CAN0.sendMsgBuf(stdID, frameLen, frameData);
  if(sndStat == CAN_OK){
    mySerial.println("Message Sent Successfully!");
  } else {
    mySerial.print("**Error Sending Message.");
    mySerial.print(sndStat);
    mySerial.print(" ");
    mySerial.print(stdID, HEX);
    mySerial.print(" ");
    mySerial.print(frameLen);
    mySerial.println();
  }
  return sndStat;
}

byte Send_Extended_frame(unsigned char frameType, byte frameLen, unsigned char *frameId, unsigned char *frameData){
  mySerial.print("Send_Extended_frame frameType=");
  mySerial.print(frameType, HEX);
  mySerial.print(" frameLength=");
  mySerial.println(frameLen);

  unsigned long extID = (frameId[3] & 0x1F);
  extID = extID << 24;
  unsigned long uid;
  uid = frameId[2];
  extID = extID + (uid<< 16);
  uid = frameId[1];
  extID = extID + (uid << 8);
  extID = extID + frameId[0];
  extID = extID | 0x80000000; // Extended Frame
  if (frameType == 0xF) extID = extID | 0x40000000; // Remote Frame

  byte sndStat = CAN0.sendMsgBuf(extID, frameLen, frameData);
  if(sndStat == CAN_OK){
    mySerial.println("Message Sent Successfully!");
  } else {
    mySerial.print("**Error Sending Message.");
    mySerial.print(sndStat);
    mySerial.print(" ");
    mySerial.print(extID, HEX);
    mySerial.print(" ");
    mySerial.print(frameLen);
    mySerial.println();
  }
  return sndStat;
}



void Receive_frame(unsigned char frameType, unsigned long rxId, unsigned char * rxBuf, unsigned char rxLen){
  unsigned char buffer[64] = {0};
  mySerial.print("Receive_frame frameType=");
  mySerial.print(frameType, HEX);
  mySerial.print(" rxLen=");
  mySerial.println(rxLen);

  buffer[0] = 0xAA;
  buffer[1] = (frameType << 4) + rxLen;
  if (frameType == 0xC || frameType == 0xD) {
    buffer[2] = rxId & 0xFF;
    buffer[3] = (rxId & 0x700) >> 8;
    for(int i=0; i<rxLen; i++){
      buffer[i+4] = rxBuf[i];
    }
    buffer[rxLen+4] = 0x55;
    for(int i=0;i<rxLen+5;i++) {
      Serial.write(buffer[i]);
    }
  } else if (frameType == 0xE || frameType == 0xF) {
    buffer[2] = rxId & 0xFF;
    buffer[3] = (rxId & 0xFF00) >> 8;
    buffer[4] = (rxId & 0xFF0000) >> 16;
    buffer[5] = (rxId & 0x1F000000) >> 24;
    for(int i=0; i<rxLen; i++){
      buffer[i+6] = rxBuf[i];
    }
    buffer[rxLen+6] = 0x55;
    for(int i=0;i<rxLen+7;i++) {
      Serial.write(buffer[i]);
    }
    
  }
}

bool isValidId(unsigned long rxId){
#if 0
  mySerial.print("isValidId config.counter=");
  mySerial.print(config.counter);
  mySerial.print(" rxId=");
  mySerial.println(rxId, HEX);
#endif
  if (config.counter == 0) return true;
  
  for(int i=0;i<config.counter;i++) {
#if 0
    mySerial.print("rxId=0x");
    mySerial.print(rxId, HEX);
    mySerial.print(" validId=0x");
    mySerial.println(config.validId[i], HEX);
#endif
    if (config.cmd == 0x10 && config.validId[i] == rxId) return true;
    if (config.cmd == 0x11 && config.validId[i] != rxId) return true;
  }
  return false;
}

void setup() {
  Serial.begin(baudrate);
  mySerial.begin(115200);
  mySerial.println("Start USB-CAN");
  lastTime = millis();
  config.counter=0;
}

void loop() {
  static unsigned char buffer[64] = {0};
  static int buflen = 0;
  static bool setupComplete = false;
  static bool startMonitor = false;
  
  unsigned long rxId;
  unsigned char rxLen = 0;
  unsigned char rxBuf[8];
  unsigned char txBuf[8];
  char msgString[128];                        // Array to store serial string

  unsigned char _cmd;
  unsigned char _len;

  byte _sndStat;
  static unsigned char recvError = 0;
  static unsigned char sendError = 0;


#if 0  
  long now = millis();
  if (now - lastTime > 1000) {
    lastTime = now;
    mySerial.print("atmega328=");
    mySerial.print(now);
    mySerial.print(" ");
    mySerial.print(recvError);
    mySerial.print(" ");
    mySerial.println(sendError);

#if 0
    buffer[0] = 0xAA;
    buffer[1] = 0x55;
    buffer[2] = 0x04;
    buffer[3] = recvError;
    buffer[4] = sendError;
    buffer[5] = sendError % 10;
    buffer[6] = sendError % 10;
    for(int i=7;i<19;i++) buffer[i] = 0x00;
    buffer[19] = calcCRC(&buffer[2], 17);
    for(int i=0;i<20;i++) {
      Serial.write(buffer[i]);
    }
#endif

    recvError++;
    sendError++;    
  }
#endif

  if (Serial.available() > 0) {
    // read the incoming byte:
    unsigned char c = Serial.read();

#if 0
    mySerial.print("c=0x");
    if (c < 0x10) mySerial.print("0");
    mySerial.println(c,HEX);
#endif

    if (analyzeCommand(c, buffer, &buflen)) {
      mySerial.print("**");
      for(int i=0; i<buflen; i++) {
        mySerial.print(buffer[i],HEX);
        mySerial.print(" ");
      }
      mySerial.println("**");
      
      if (buffer[1] == 0x55) {
        if (buffer[2] == 0x1) { // Fixed 20 bytes to send
          if (buffer[3] == 1) { // Standard Frame
            _cmd = 0xB + buffer[4];
            _sndStat = Send_Standard_frame(_cmd, buffer[9], &buffer[5], &buffer[10]);
            if (_sndStat != CAN_OK) sendError++;
          } else {
            _cmd = 0xD + buffer[4];
            _sndStat = Send_Extended_frame(_cmd, buffer[9], &buffer[5], &buffer[10]);
            if (_sndStat != CAN_OK) sendError++;
          }
        }
        
        if (buffer[2] == 0x4) {
          Monitor(recvError, sendError, buffer, buflen);
          //startMonitor = !startMonitor;
        }

        if (buffer[2] == 0x06 || buffer[2] == 0x13
         || buffer[2] == 0x30 || buffer[2] == 0x32 
         || buffer[2] == 0x33 || buffer[2] == 0x34
         || buffer[2] == 0x07) {
          mySerial.println(NOT_SUPPORT);
        }

        if (buffer[2] == 0x10 || buffer[2] == 0x11) { // Configure the receive ID
          Configure_Receive_ID(buffer, buflen);
        }

        if (buffer[2] == 0x12) {
          Set_and_Start(buffer, buflen);
          setupComplete = true;
          startMonitor = true;
        }

      } else {
        _cmd = (buffer[1] & 0xF0) >> 4;
        _len = (buffer[1] & 0x0F);
        if (_cmd == 0xC || _cmd == 0xD) { // Standard Frame
          _sndStat = Send_Standard_frame(_cmd, _len, &buffer[2], &buffer[4]);
          if (_sndStat != CAN_OK) sendError++;
        }
        if (_cmd == 0xE || _cmd == 0xF) { // Extended Frame
          _sndStat = Send_Extended_frame(_cmd, _len, &buffer[2], &buffer[6]);
          if (_sndStat != CAN_OK) sendError++;
        }
        
      }


    }
  } // end Serial.available

  if (setupComplete) {
    if (!digitalRead(CAN0_INT))                 // If CAN0_INT pin is low, read receive buffer
    {
      CAN0.readMsgBuf(&rxId, &rxLen, rxBuf);      // Read data: rxLen = data length, rxBuf = data byte(s)

#if 0
      mySerial.print("rxId=0x");
      mySerial.print(rxId, HEX);
      mySerial.print(" rxLen=");
      mySerial.print(rxLen);
      mySerial.println();
#endif      

      unsigned char frameType;
      if((rxId & 0x80000000) == 0x80000000) {   // Determine if ID is standard (11 bits) or extended (29 bits)
        sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), rxLen);
        //frameType = 0xE8; // Extended Data frame
        frameType = 0xE; // Extended Data frame
      } else {
        sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, rxLen);
        //frameType = 0xC8; // Standard Data frame
        frameType = 0xC; // Extended Data frame
      }
      mySerial.print(msgString);
    
      if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
        sprintf(msgString, " REMOTE REQUEST FRAME");
        frameType = frameType + 0x1;
        mySerial.print(msgString);
      } else {
        for(byte i = 0; i<rxLen; i++){
          sprintf(msgString, " 0x%.2X", rxBuf[i]);
          mySerial.print(msgString);
        }
      }
      mySerial.println();
      if (startMonitor) {
        rxId = (rxId & 0x1FFFFFFF);
        if (isValidId(rxId)) Receive_frame(frameType, rxId, rxBuf, rxLen);
      } else {
        //mySerial.println("Monitor not started");
      }
    }
  }

  
}
