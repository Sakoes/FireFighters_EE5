#include <SPI.h>
#include <LoRa.h>
#include <Nextion.h>
#define LED1   A0
#define LED2   A1
#define LED3   A2     //A2
#define ACKBUT A3
#define BUZZER 5

enum decimal {
  NO,
  CURRENT,
  SET
};



int gas[4] = {0, 21, 0, 0}; //CH4 O2 CO IBUT
decimal gasPoint[4];

char val[50] = {0};

byte localAddress = 0xBB;     // address of this device
byte destination = 0xBC;
unsigned long rssiMillis;

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 500 ; //the period of blinking LEDs/buzzer


int tres[8] = {10,20,19,23,20,100,100,200};
int tresPoint[8] = {NO,NO,NO,NO,NO,NO,NO,NO};
const unsigned int alarmColor1 = 63488;
const unsigned int alarmColor2 = 64512;
const unsigned int neutralColor = 65535;

boolean  alarmFlag1 =   false; //buzzer on when true
boolean  alarmFlag2 =   false;


union  // saving Latitude
{
  float flat;
  byte bytelat[4];
} gps_lat;

union  //saving Lon
{
  float flon;
  byte bytelon[4];
} gps_lon;


void serialEnd() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void sendData() {
  if (millis() > rssiMillis + 1000) {
    //These pointers will point to the first byte of both floats
    //byte *lon = (byte *)&longitude;
    //byte *lat = (byte *)&latitude;
    LoRa.beginPacket();
    LoRa.write(destination);
    // for (int i = 0; i < 4; i++) {
    //   LoRa.write(gps_lat.bytelat[i]);
    // }
    // for (int i = 0; i < 4; i++) {
    //   LoRa.write(gps_lon.bytelon[i]);
    // }
    // for(int i = 0; i < 4; i++){ //Write longitude
    //   LoRa.write(*(lon+i));
    // }
    // for(int i = 0; i < 4; i++){ //Write latitude
    //   LoRa.write(*(lat+i));
    // }
    LoRa.endPacket();
    rssiMillis = 0;
    rssiMillis = millis();
    //Serial.print("gas1.bco=0");
  }
}


void printData() {
  serialEnd();
  for (int i = 0; i < 4; i++) {
    if (gasPoint[i] == SET) {
      int t1 = gas[i] / 10;
      int t2 = gas[i] - (gas[i] / 10) * 10;
      sprintf(val, "gas%ivalue.txt=\"%i,%i\"", i + 1, t1, t2);
      Serial.print(val);
      serialEnd();
    }
    else {
      sprintf(val, "gas%ivalue.txt=\"%i\"", i + 1, gas[i]);
      Serial.print(val);
      serialEnd();
    }
  }
}



void setup() {
  Serial.begin(9600);
  gps_lat.flat = 50.874845;
  gps_lon.flon = 4.707942;

  pinMode(LED1,  OUTPUT);
  pinMode(LED2,  OUTPUT);
  pinMode(LED3,  OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(ACKBUT, INPUT);

  startMillis = millis();
  rssiMillis = millis();


  while (!Serial);
  //Serial.println("LoRa Receiver");
  LoRa.setPins(10, 9, 2);
  //LoRa.setSPIFrequency(8E6);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  nexInit();
}


void loop() {
  loraReceive();

  //Button pressed: turn off alarms
  if(digitalRead(ACKBUT) == HIGH){
    alarmFlag1 = false;
    alarmFlag2 = false;
  }

  //This function will check if an alarmFlag is true and will start the alarm if so
  alarm();

  //An empty LoRa packet is sent to the instructor device, for signal strength
  sendData();
}

// void tresReceive(){
//   // try to parse packet
//   if (LoRa.parsePacket()) {
//     while (LoRa.available()) {
//       if (LoRa.read() == localAddress && LoRa.read() == 0x00) {
//         for(int i = 0; i < 4; i++){
//           gas[i] = word(LoRa.read(), LoRa.read());
//           gasPoint[i] = word(LoRa.read(), LoRa.read());
//         }
//
//         LoRa.beginPacket();
//         LoRa.write(destination);
//         for(int i = 0; i < 4; i++){
//           LoRa.write(lowByte(gas[i]));
//           LoRa.write(highByte(gas[i]));
//           LoRa.write(lowByte(gasPoint[i]));
//           LoRa.write(highByte(gasPoint[i]));
//         }
//         LoRa.endPacket();
// }


void loraReceive(){
  if(LoRa.parsePacket()){
    while(LoRa.available()){
      if(LoRa.read() == localAddress){
        if(LoRa.read() == 0xFF){  //GAS VALUES
          for(int i = 0; i < 4; i++){
            gas[i] = word(LoRa.read(), LoRa.read());
            gasPoint[i] = word(LoRa.read(), LoRa.read());
          }
          LoRa.beginPacket();
          LoRa.write(destination);
          LoRa.write(0xFF)
          for(int i = 0; i < 4; i++){
            LoRa.write(lowByte(gas[i]));
            LoRa.write(highByte(gas[i]));
            LoRa.write(lowByte(gasPoint[i]));
            LoRa.write(highByte(gasPoint[i]));
          }
          LoRa.endPacket();

          printData();

          //Clear ackflag
          checkGasses();
        }

        else if(LoRa.read() == 0x00){ //THRESHOLD VALUES
          for(int i = 0; i < 8; i++){
            tres[i] = word(LoRa.read(), LoRa.read());
            tresPoint[i] = word(LoRa.read(), LoRa.read());
          }
          LoRa.beginPacket();
          LoRa.write(destination);
          LoRa.write(0x00)
          for(int i = 0; i < 8; i++){
            LoRa.write(lowByte(tres[i]));
            LoRa.write(highByte(tres[i]));
            LoRa.write(lowByte(tresPoint[i]));
            LoRa.write(highByte(tresPoint[i]));
          }
          LoRa.endPacket();
        }
      }
    }
  }
}



// void loraReceive(){
//   // try to parse packet
//   if (LoRa.parsePacket()) {
//     while (LoRa.available()) {
//       if(LoRa.read() == localAddress){
//         if(LoRa.read() == 0xFF){}
//       }
//
//
//
//       if (LoRa.read() == localAddress && LoRa.read() == 0xff) {
//
//         for(int i = 0; i < 4; i++){
//           gas[i] = word(LoRa.read(), LoRa.read());
//           gasPoint[i] = word(LoRa.read(), LoRa.read());
//         }
//
//
//         //Sending Acknowledgement
//         //LoRa.beginPacket();
//         //LoRa.write(destination);
//         //LoRa.write(0xFF);//Acknowledgement
//         //LoRa.endPacket();
//         LoRa.beginPacket();
//         LoRa.write(destination);
//         for(int i = 0; i < 4; i++){
//           LoRa.write(lowByte(gas[i]));
//           LoRa.write(highByte(gas[i]));
//           LoRa.write(lowByte(gasPoint[i]));
//           LoRa.write(highByte(gasPoint[i]));
//         }
//         LoRa.endPacket();
//
//
//         printData();
//
//         //Clear ackflag
//         checkGasses();
//       }
//     }
//
//     // print RSSI of packet
//     sprintf(val, "lora.txt=\"%i\"", LoRa.packetRssi());
//     Serial.print(val);
//     serialEnd();
//   }
// }

void checkGasses(){
  for(int i = 0; i < 4; i++){
    switch (i) {
      case 1: //O2
        if(gas[i] >= tres[2*i+1]){
          alarmFlag2 = true;
          setAlarmBackground(i+1, 1);
        }
        else if(gas[i] <= tres[2*i]){
          alarmFlag1 = true;
          setAlarmBackground(i+1, 1);
        }
        else{
          setAlarmBackground(i+1, 0);
        }
        break;

      default: //All other gasses
        if(gas[i] >= tres[2*i+1]){
          alarmFlag2 = true;
          setAlarmBackground(i+1, 1);
        }
        else if(gas[i] >= tres[2*i]){
          alarmFlag1 = true;
          setAlarmBackground(i+1, 2);
        }
        else{
          setAlarmBackground(i+1, 0);
        }
        break;
    }
  }
}

void setAlarmBackground(int gas, unsigned int color){ // color 0 = neutral, 1 = red, 2 = orange
  sprintf(val, "dataFlag.val=%i", 1);
  Serial.print(val);
  serialEnd();
  sprintf(val, "gas%iflag.val=%u", gas, color);
  Serial.print(val);
  serialEnd();
}

void alarm() {
  currentMillis = millis(); //get the current time (number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)
  {
    if (alarmFlag1 == true || alarmFlag2 == true) {
      digitalWrite(LED1, !digitalRead(LED1));   //blinking LEds
      digitalWrite(LED2, !digitalRead(LED2));
      digitalWrite(LED3, !digitalRead(LED3));
      if (alarmFlag2 == true) {
        tone(BUZZER, 440, 250);
      }
      else {
        tone(BUZZER, 261, 250 );
      }
      startMillis = currentMillis;
    }
    else {
      noTone(BUZZER);
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
    }
  }
}
