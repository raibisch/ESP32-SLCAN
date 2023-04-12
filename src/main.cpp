#include <Arduino.h>
#include <Preferences.h>             // by JG store Serial-Baudrate
#include <ESP32CAN.h>           // v1.0.0     from https://github.com/nhatuan84/arduino-esp32-can-demo
#include <CAN_config.h>         // as above

// #define BT_SERIAL
#ifdef BT_SERIAL
#include "BluetoothSerial.h"    // v1.0
#endif

// CURRENTLY ESP32 Dev Module Board Definition
// PIN 4  CANTX 
// PIN 5  CANRX 
// by JG change: CANTX=5, CANRX=4
// PIN 12 BLUETOOTH SWITCH (only if BT_SERIAL defined)
// PIN 14 NOT IN USE
// PIN 15 10k to ground to remove boot messages
// 3.3v 
// GND to SWITCH CENTER

CAN_device_t              CAN_cfg;
#ifdef BT_SERIAL
BluetoothSerial           SerialBT;
#endif

Preferences prefs;        // by JG

boolean working           = false;
boolean timestamp         = false;
boolean cr                = false;
int can_speed             = 250;
static int ser_speed      = 115200;

#ifdef BT_SERIAL
boolean bluetooth = false;
const int SWITCH_PIN_A    = 12;
#endif
//const int SWITCH_PIN_B  = 14;

static uint8_t hexval[17] = "0123456789ABCDEF";

//----------------------------------------------------------------

void slcan_ack()
{
  #ifdef BT_SERIAL
  if (bluetooth) SerialBT.write('\r');
  else 
  #endif
  Serial.write('\r');
} // slcan_ack()

//----------------------------------------------------------------

void slcan_nack()
{
  #ifdef BT_SERIAL
  if (bluetooth) SerialBT.write('\a');
  else 
  #endif
  Serial.write('\a');
} // slcan_nack()

//----------------------------------------------------------------

void send_canmsg(char *buf, boolean rtr, boolean ext) {
  if (!working) {

  } else {
    CAN_frame_t tx_frame;
    int msg_id = 0;
    int msg_ide = 0;
    if (rtr) {
      if (ext) {
        sscanf(&buf[1], "%04x%04x", &msg_ide, &msg_id);
        tx_frame.FIR.B.RTR = CAN_RTR;
        tx_frame.FIR.B.FF = CAN_frame_ext;
      } else {
        sscanf(&buf[1], "%03x", &msg_id);
        tx_frame.FIR.B.RTR = CAN_RTR;
        tx_frame.FIR.B.FF = CAN_frame_std;
      }
    } else {
      if (ext) {
        sscanf(&buf[1], "%04x%04x", &msg_ide, &msg_id);
        tx_frame.FIR.B.RTR = CAN_no_RTR;
        tx_frame.FIR.B.FF = CAN_frame_ext;
      } else {
        sscanf(&buf[1], "%03x", &msg_id);
        tx_frame.FIR.B.RTR = CAN_no_RTR;
        tx_frame.FIR.B.FF = CAN_frame_std;
      }
    }
    tx_frame.MsgID = msg_ide*65536 + msg_id;
    int msg_len = 0;
    if (ext) {
      sscanf(&buf[9], "%01x", &msg_len);
    } else {
      sscanf(&buf[4], "%01x", &msg_len);
    }
    tx_frame.FIR.B.DLC = msg_len;
    int candata = 0;
    if (ext) {
      for (int i = 0; i < msg_len; i++) {
        sscanf(&buf[10 + (i*2)], "%02x", &candata);
        tx_frame.data.u8[i] = candata;
      }
    } else {
      for (int i = 0; i < msg_len; i++) {
        sscanf(&buf[5 + (i*2)], "%02x", &candata);
        tx_frame.data.u8[i] = candata;
      }
    }
    ESP32Can.CANWriteFrame(&tx_frame);
  }
} // send_canmsg()


//----------------------------------------------------------------

void pars_slcancmd(char *buf)
{                           // LAWICEL PROTOCOL
  switch (buf[0]) 
  {
    case 'O':               // OPEN CAN
      working=true;
      ESP32Can.CANInit();
      slcan_ack();
      break;
    case 'C':               // CLOSE CAN
      working=false;
      ESP32Can.CANStop();
      slcan_ack();
      break;
    case 't':               // SEND STD FRAME
      send_canmsg(buf,false,false);
      Serial.write('z'); // by JG: is send with original CANUSB-Adapter V1011
      slcan_ack();
      break;
    case 'T':               // SEND EXT FRAME
      send_canmsg(buf,false,true);
      Serial.write('Z'); // by JG: is send with original CANUSB-Adapter V1011
      slcan_ack();
      break;
    case 'r':               // SEND STD RTR FRAME
      send_canmsg(buf,true,false);
      slcan_ack();
      break;
    case 'R':               // SEND EXT RTR FRAME
      send_canmsg(buf,true,true);
      slcan_ack();
      break;
    case 'Z':               // ENABLE TIMESTAMPS
      switch (buf[1]) {
        case '0':           // TIMESTAMP OFF  
          timestamp = false;
          slcan_ack();
          break;
        case '1':           // TIMESTAMP ON
          timestamp = true;
          slcan_ack();
          break;
        default:
          break;
      }
      break;
    case 'M': // set ACCEPTANCE CODE ACn REG (not supported)
      slcan_ack();
      break;
    case 'm': // set ACCEPTANCE CODE AMn REG (not supported)
      slcan_ack();
      break;
    case 's': // CUSTOM CAN bit-rate (not supported)
      slcan_nack();
      break;
    case 'S':               // CAN bit-rate
      switch (buf[1]) {
        case '0':           // 10k  
          slcan_nack();
          break;
        case '1':           // 20k
          slcan_nack();
          break;
        case '2':           // 50k
          slcan_nack();
          break;
        case '3':           // 100k
          CAN_cfg.speed=CAN_SPEED_100KBPS;
          can_speed = 100;
          slcan_ack();
          break;
        case '4':           // 125k
          CAN_cfg.speed=CAN_SPEED_125KBPS;
          can_speed = 125;
          slcan_ack();
          break;
        case '5':           // 250k
          CAN_cfg.speed=CAN_SPEED_250KBPS;
          can_speed = 250;
         slcan_ack();
          break;
        case '6':           // 500k
          CAN_cfg.speed=CAN_SPEED_500KBPS;
          can_speed = 500;
          slcan_ack();
          break;
        case '7': // 800k
          CAN_cfg.speed=CAN_SPEED_800KBPS;
          can_speed = 800;
          slcan_ack();
          break;
        case '8':           // 1000k
          CAN_cfg.speed=CAN_SPEED_1000KBPS;
          can_speed = 1000;
          slcan_ack();
          break;
        default:
          slcan_nack();
          break;
      }
      break;
    case 'F':               // STATUS FLAGS
      #ifdef BT_SERIAL
      if (bluetooth) SerialBT.print("F00");
      else 
      #endif
      Serial.write('F');
      Serial.print("00");  // Fake output no real value
      slcan_ack();
      break;
    case 'V':               // VERSION NUMBER
      #ifdef BT_SERIAL
      if (bluetooth) SerialBT.print("V1234");
      else 
      #endif
      Serial.write('V');
      Serial.print("1012");
      slcan_ack();
      break;
    case 'N':               // SERIAL NUMBER
       #ifdef BT_SERIAL
      if (bluetooth) SerialBT.print("N2208");
      else 
      #endif
      Serial.write('N');
      Serial.print("Z911");
      slcan_ack();
      break;
    case 'b':               // by JG: (NOT SPEC) Set Serial-Baud (0) 115200 or (1) 460800
      if (buf[1] == '1')
      {
        ser_speed = 460800;
        prefs.putChar("bx", 1);
      }
      else
      {
       ser_speed = 115200;
       prefs.putChar("bx", 0);
      }
      Serial.print("set Baudrate to:");
      Serial.println(ser_speed);
      slcan_nack();
      delay(100);
      Serial.updateBaudRate(ser_speed);
      break;
    case 'x':
      ESP.restart();
      break;
    case 'l':               // (NOT SPEC) TOGGLE LINE FEED ON SERIAL
      cr = !cr;
      slcan_nack();
      break;
    case 'h':               // (NOT SPEC) HELP SERIAL
      Serial.println();
      Serial.println("slcan esp32");
      Serial.println();
      Serial.println("O  = Start slcan");
      Serial.println("C  = Stop slcan");
      Serial.println("t  = Send std frame");
      Serial.println("r  = Send std rtr frame");
      Serial.println("T  = Send ext frame");
      Serial.println("R  = Send ext rtr frame");
      Serial.println("Z0 = Timestamp Off");
      Serial.println("Z1 = Timestamp On");
      Serial.println("snn= Speed 0xnnk N/A");
      Serial.println("S0 = Speed 10k N/A");
      Serial.println("S1 = Speed 20k N/A");
      Serial.println("S2 = Speed 50k N/A");
      Serial.println("S3 = Speed 100k");
      Serial.println("S4 = Speed 125k");
      Serial.println("S5 = Speed 250k");
      Serial.println("S6 = Speed 500k");
      Serial.println("S7 = Speed 800k");
      Serial.println("S8 = Speed 1000k");
      Serial.println("F  = Flags N/A");
      Serial.println("N  = Serial No");
      Serial.println("V  = Version");
      Serial.println("-----NOT SPEC-----");
      Serial.println("h  = this Help");
      Serial.println("b0 = SerBaud:115200");
      Serial.println("b1 = SerBaud:460800");
      Serial.println("x  = RESTART ESP32");
      Serial.print("l  = Toggle CR ");
      if (cr) {
        Serial.println("ON");
      } else {
        Serial.println("OFF");
      }
      Serial.print("CAN_SPEED:");
      switch(can_speed) {
        case 100:
          Serial.print("100");
          break;
        case 125:
          Serial.print("125");
          break;
        case 250:
          Serial.print("250");
          break;
        case 500:
          Serial.print("500");
          break;
        case 800:
          Serial.print("800");
          break;
        case 1000:
          Serial.print("1000");
          break;
        default:
          break;
      }
      Serial.print("kbps ");
      if (timestamp) {
        Serial.print("TStamp:ON ");
      }
      if (working) {
        Serial.print("CAN:OPEN");
      } else {
        Serial.print("CAN:CLOSE");
      }
      Serial.println();
      slcan_nack();
      break;
    default:
      slcan_nack();
      break;
  }
} // pars_slcancmd()

//----------------------------------------------------------------

void transfer_tty2can()
{
  int ser_length;
  static char cmdbuf[32];
  static int cmdidx = 0;
  #ifdef BT_SERIAL
  if (bluetooth) 
  {
    if ((ser_length = SerialBT.available()) > 0) {
      for (int i = 0; i < ser_length; i++) {
        char val = SerialBT.read();
        cmdbuf[cmdidx++] = val;
        if (cmdidx == 32)
        {
          slcan_nack();
          cmdidx = 0;
        } else if (val == '\r')
        {
          cmdbuf[cmdidx] = '\0';
          pars_slcancmd(cmdbuf);
          cmdidx = 0;
        }
      }
    }
  } 
  else 
  #endif
  {
    if ((ser_length = Serial.available()) > 0) 
    {
      for (int i = 0; i < ser_length; i++) 
      {
        char val = Serial.read();
        cmdbuf[cmdidx++] = val;
        if (cmdidx == 32)
        {
          slcan_nack();
          cmdidx = 0;
        } else if (val == '\r')
        {
          cmdbuf[cmdidx] = '\0';
          pars_slcancmd(cmdbuf);
          cmdidx = 0;
        }
      }
    }
  }
} // transfer_tty2can()

//----------------------------------------------------------------

void transfer_can2tty()
{
  CAN_frame_t rx_frame;
  String command = "";
  long time_now = 0;
  //receive next CAN frame from queue
  // alternative implementation (not testet)
  /*
    // Abfrage von Empfangenen CAN-Nachhrichten
    if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) 
    {
      // Abfrage auf Ext. Datenframe
      if (rx_frame.FIR.B.FF == CAN_frame_ext) 
      {
        ASCIIFrame[0] = 84; // -> 'T'
        byteToHex(rx_frame.MsgID >> 24, &ASCIIFrame[1]);
        byteToHex(rx_frame.MsgID >> 16, &ASCIIFrame[3]);
        byteToHex(rx_frame.MsgID >> 8, &ASCIIFrame[5]);
        byteToHex(rx_frame.MsgID , &ASCIIFrame[7]);
        ASCIIFrame[9] = nibbleToHex(rx_frame.FIR.B.DLC);
        for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
          byteToHex(rx_frame.data.u8[i], &DataASCII[0]);
          ASCIIFrame[10+(2*i)]=DataASCII[0];
          ASCIIFrame[11+(2*i)]=DataASCII[1];
        }
        // Wiedergabe ohne Timestamp
        // [CR]
        ASCIIFrame[10+(rx_frame.FIR.B.DLC * 2)]=13;
        // Ausgabe des Frames, Laenge: 15...31 Bytes
        Serial.write(ASCIIFrame, (11+(rx_frame.FIR.B.DLC * 2)));
      } // Ende Ext. Dataframe
    }// Ende CAN-Empfang

  */

  if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE) 
  {
    //do stuff!
    if(working) 
    {
      if(rx_frame.FIR.B.FF==CAN_frame_ext) 
      {
        if (rx_frame.FIR.B.RTR==CAN_RTR) 
        {
          command = command + "R";
        } else 
        {
          command = command + "T";
        }
        command = command + char(hexval[ (rx_frame.MsgID>>28)&1]);
        command = command + char(hexval[ (rx_frame.MsgID>>24)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>20)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>16)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>12)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>8)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>4)&15]);
        command = command + char(hexval[ rx_frame.MsgID&15]);
        command = command + char(hexval[ rx_frame.FIR.B.DLC ]);
      } 
      else 
      {
        if (rx_frame.FIR.B.RTR==CAN_RTR) {
          command = command + "r";
        } else {
          command = command + "t";
        }
        command = command + char(hexval[ (rx_frame.MsgID>>8)&15]);
        command = command + char(hexval[ (rx_frame.MsgID>>4)&15]);
        command = command + char(hexval[ rx_frame.MsgID&15]);
        command = command + char(hexval[ rx_frame.FIR.B.DLC ]);
      }
      for(int i = 0; i < rx_frame.FIR.B.DLC; i++){
        command = command + char(hexval[ rx_frame.data.u8[i]>>4 ]);
        command = command + char(hexval[ rx_frame.data.u8[i]&15 ]);
        //printf("%c\t", (char)rx_frame.data.u8[i]);
      }
    if (timestamp) 
    {
      time_now = millis() % 60000;
      command = command + char(hexval[ (time_now>>12)&15 ]);
      command = command + char(hexval[ (time_now>>8)&15 ]);
      command = command + char(hexval[ (time_now>>4)&15 ]);
      command = command + char(hexval[ time_now&15 ]);
    }
    command = command + '\r';
     #ifdef BT_SERIAL
    if (bluetooth) SerialBT.print(command);
    else 
    #endif
    Serial.print(command);
    if (cr) Serial.println("");
    }
  }
} // transfer_can2tty()


//----------------------------------------------------------------

void setup() 
{
  delay(200);
  prefs.begin("pref");
  uint8_t bx= prefs.getChar("bx");
  if (bx == 1)
  {
    ser_speed = 460800;
    prefs.putChar("bx",1);
  }  
  else
  {
    ser_speed = 115200;
    prefs.putChar("bx",0);
  }
  
 
#ifdef BT_SERIAL
  pinMode(SWITCH_PIN_A,INPUT_PULLUP);
#endif
  //pinMode(SWITCH_PIN_B,INPUT_PULLUP);
  Serial.begin(ser_speed);
  delay(100);
  Serial.print("set Baudrate to:");
  Serial.println(ser_speed);

  //Serial.println("CAN demo");
  CAN_cfg.speed=CAN_SPEED_250KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
  delay(2000);
   #ifdef BT_SERIAL
  boolean switchA = digitalRead(SWITCH_PIN_A);

  if (!switchA) {
    SerialBT.begin("SLCAN");
    bluetooth = true;
    Serial.println("BT Switch ON");
  } else {
    bluetooth = false;
    Serial.println("BT Switch OFF");
  }
  if (bluetooth) Serial.println("BLUETOOTH ON");
  #endif
} // setup()

//----------------------------------------------------------------

void loop() 
{
  transfer_can2tty();
  NOP();
  //delayMicroseconds(1);
  transfer_tty2can();
} // loop();
