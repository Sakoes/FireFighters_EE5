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

int gas[3] = {0, 21, 0}; //CH4 O2 CO
decimal gasPoint[3];

char val[50] = {0};

byte localAddress = 0xBB;     // address of this device
byte destination = 0xBC;
unsigned long rssiMillis;

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 500 ; //the period of blinking LEDs/buzzer

//Gas alarm values as constants
const int gas0A1 = 10;
const int gas0A2 = 20;
const int gas1A1 = 19;
const int gas1A2 = 23;
const int gas2A1 = 20;
const int gas2A2 = 100;


boolean  alarmFlag1 =   false; //buzzer on when true
boolean  alarmFlag2 =   false;


void serialEnd() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void sendData() {
  if (millis() > rssiMillis + 1000) {
    LoRa.beginPacket();
    LoRa.write(destination);
    LoRa.endPacket();
    rssiMillis = 0;
    rssiMillis = millis();
    //Serial.print("gas1.bco=0");
  }
}


void printData() {
  serialEnd();
  for (int i = 0; i < 3; i++) {
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
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet

    // read packet
    while (LoRa.available()) {
      if (LoRa.read() == localAddress) {
        gas[0] = word(LoRa.read(), LoRa.read());
        gasPoint[0] = LoRa.read();
        gas[1] = word(LoRa.read(), LoRa.read());
        gasPoint[1] = LoRa.read();
        gas[2] = word(LoRa.read(), LoRa.read());
        gasPoint[2] = LoRa.read();
        //counter = LoRa.read();
        printData();
        //LoRa.endPacket();
        // clear ackflag
        checkGasses();
      }
      //Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    sprintf(val, "lora.txt=\"%i\"", LoRa.packetRssi());
    Serial.print(val);
    serialEnd();
    //Serial.println(LoRa.packetRssi());
    //debugging();
  }

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



void checkGasses(){
  //Explosives check
  if(gas[0] >= gas0A2){
    alarmFlag2 = true;
  }
  else if(gas[0] >= gas0A1){
    alarmFlag1 = true;
  }

  //O2 check
  if(gas[1] >= gas1A2){
    alarmFlag2 = true;
  }
  else if(gas[1] <= gas1A1){
    alarmFlag1 = true;
  }

  //CO check
  if(gas[2] >= gas2A2){
    alarmFlag2 = true;
  }
  else if(gas[2] >= gas2A1){
    alarmFlag1 = true;
  }
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
