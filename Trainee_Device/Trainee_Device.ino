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



const int tresHolds[8] = {10,20,19,23,20,100,100,200};
const unsigned int alarmColor1 = 63488;
const unsigned int alarmColor2 = 64512;
const unsigned int neutralColor = 65535;

boolean  alarmFlag1 =   false; //buzzer on when true
boolean  alarmFlag2 =   false;


float latitude = 1.2345;
float longitude = 5.4321;


void serialEnd() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void sendData() {
  if (millis() > rssiMillis + 1000) {
    //These pointers will point to the first byte of both floats
    byte *lon = (byte *)&longitude;
    byte *lat = (byte *)&latitude;
    LoRa.beginPacket();
    LoRa.write(destination);
    for(int i = 0; i < 4; i++){ //Write longitude
      LoRa.write(*(lon+i));
    }
    for(int i = 0; i < 4; i++){ //Write latitude
      LoRa.write(*(lat+i));
    }
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

void loraReceive(){
  // try to parse packet
  if (LoRa.parsePacket()) {
    while (LoRa.available()) {
      if (LoRa.read() == localAddress) {
        for(int i = 0; i < 4; i++){
          gas[i] = word(LoRa.read(), LoRa.read());
          gasPoint[i] = LoRa.read();
        }

        //Sending Acknowledgement
        LoRa.beginPacket();
        LoRa.write(destination);
        LoRa.write(0xFF);//Acknowledgement
        LoRa.endPacket();


        printData();

        //Clear ackflag
        checkGasses();
      }
    }

    // print RSSI of packet
    sprintf(val, "lora.txt=\"%i\"", LoRa.packetRssi());
    Serial.print(val);
    serialEnd();
  }
}

void checkGasses(){
  for(int i = 0; i < 4; i++){
    switch (i) {
      case 1: //O2
        if(gas[i] >= tresHolds[2*i+1]){
          alarmFlag2 = true;
          setAlarmBackground(i+1, alarmColor1);
        }
        else if(gas[i] <= tresHolds[2*i]){
          alarmFlag1 = true;
          setAlarmBackground(i+1, alarmColor1);
        }
        else{
          setAlarmBackground(i+1, neutralColor);
        }
        break;

      default: //All other gasses
        if(gas[i] >= tresHolds[2*i+1]){
          alarmFlag2 = true;
          setAlarmBackground(i+1, alarmColor1);
        }
        else if(gas[i] >= tresHolds[2*i]){
          alarmFlag1 = true;
          setAlarmBackground(i+1, alarmColor2);
        }
        else{
          setAlarmBackground(i+1, neutralColor);
        }
        break;
    }
  }
}

void setAlarmBackground(int gas, unsigned int color){
  sprintf(val, "gas%i.bco=%u", gas, color);
  Serial.print(val);
  serialEnd();
  sprintf(val, "gas%ivalue.bco=%u", gas, color);
  Serial.print(val);
  serialEnd();
  sprintf(val, "unit%i.bco=%u", gas, color);
  Serial.print(val);
  serialEnd();
  // sprintf(val, "b%i.bco=%u", gas, color);
  // Serial.print(val);
  // serialEnd();
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
