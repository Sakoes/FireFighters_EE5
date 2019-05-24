#include <SPI.h>
#include <LoRa.h>
#include <Nextion.h>
#define LED1   A0
#define LED2   A1
#define LED3   A2     //A2
#define ACKBUT A3
#define TOGGLEBUT A4
#define BATTERY A5
#define BUZZER 5

enum decimal {
  NO,
  CURRENT,
  SET
};


NexText gas1Text = NexText(0, 5, "gas1");
NexText gas2Text = NexText(0, 6, "gas2");
NexText gas3Text = NexText(0, 7, "gas3");
NexText gas4Text = NexText(0, 8, "gas4");
NexText gas1ValueText = NexText(0, 14, "gas1value");
NexText gas2ValueText = NexText(0, 15, "gas2value");
NexText gas3ValueText = NexText(0, 16, "gas3value");
NexText gas4ValueText = NexText(0, 17, "gas4value");
NexText unit1Text = NexText(0, 9, "unit1");
NexText unit2Text = NexText(0, 10, "unit2");
NexText unit3Text = NexText(0, 12, "unit3");
NexText unit4Text = NexText(0, 11, "unit4");
NexButton b1Button = NexButton(0, 4, "b1");
NexButton b2Button = NexButton(0, 3, "b2");
NexButton b3Button = NexButton(0, 2, "b3");
NexButton b4Button = NexButton(0, 1, "b4");

NexButton backButton1 = NexButton(1, 11, "backBtn");
NexButton backButton2 = NexButton(2, 11, "backBtn");
NexButton backButton3 = NexButton(3, 11, "backBtn");
NexButton backButton4 = NexButton(4, 11, "backBtn");


NexTouch *nex_listen_list[] =
{
  &gas1Text,
  &gas2Text,
  &gas3Text,
  &gas4Text,
  &gas1ValueText,
  &gas2ValueText,
  &gas3ValueText,
  &gas4ValueText,
  &unit1Text,
  &unit2Text,
  &unit3Text,
  &unit4Text,
  &b1Button,
  &b2Button,
  &b3Button,
  &b4Button,
  &backButton1,
  &backButton2,
  &backButton3,
  &backButton4,
  NULL
};  // End of touch event list

int page = 0;
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
boolean toggleDebounce = false;

unsigned int lastBatteryMeasurement;


void serialEnd() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void showThresholds1() {
  page = 1;
  showThresholds(1);
}

void showThresholds2() {
  page = 2;
  showThresholds(2);
}

void showThresholds3() {
  page = 3;
  showThresholds(3);
}

void showThresholds4() {
  page = 4;
  showThresholds(4);
}

void showThresholds(int num) {
  if(tresPoint[2*(num-1)] == SET) {
    sprintf(val, "treshold1.txt=\"%i,%i\"", tres[2*(num-1)] / 10, tres[2*(num-1)] - (tres[2*(num-1)] / 10) * 10);
    Serial.print(val);
    serialEnd();
  }
  else{
    sprintf(val, "treshold1.txt=\"%i\"", tres[2*(num-1)]);
    Serial.print(val);
    serialEnd();
  }

  if(tresPoint[2*(num-1)+1] == SET) {
    sprintf(val, "treshold2.txt=\"%i,%i\"", tres[2*(num-1)+1] / 10, tres[2*(num-1)+1] - (tres[2*(num-1)+1] / 10) * 10);
    Serial.print(val);
    serialEnd();
  }
  else{
    sprintf(val, "treshold2.txt=\"%i\"", tres[2*(num-1)+1]);
    Serial.print(val);
    serialEnd();
  }
}


void sendData() {
  if (millis() > rssiMillis + 1000) {
    LoRa.beginPacket();
    LoRa.write(destination);
    LoRa.endPacket();
    rssiMillis = millis();
  }
}


void printData() {
  serialEnd();
  for (int i = 0; i < 4; i++) {
    if (gasPoint[i] == SET) {
      sprintf(val, "gas%ivalue.txt=\"%i,%i\"", i + 1, gas[i] / 10, gas[i] - (gas[i] / 10) * 10);
      Serial.print(val);
      serialEnd();
    }
    else {
      sprintf(val, "gas%ivalue.txt=\"%i\"", i + 1, gas[i]);
      Serial.print(val);
      serialEnd();
    }
  }
  page = 0;
}

void updatePage(){
  if(page == 0)
    printData();
  else if(page == 1)
    showThresholds1();
  else if(page == 2)
    showThresholds2();
  else if(page == 3)
    showThresholds3();
  else if(page == 4)
    showThresholds4();
}

void setup() {
  Serial.begin(9600);

  pinMode(LED1,  OUTPUT);
  pinMode(LED2,  OUTPUT);
  pinMode(LED3,  OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(ACKBUT, INPUT);
  pinMode(TOGGLEBUT, INPUT);

  startMillis = millis();
  rssiMillis = millis();

  nexInit();

  gas1Text.attachPop(showThresholds1, &gas1Text);
  gas2Text.attachPop(showThresholds2, &gas2Text);
  gas3Text.attachPop(showThresholds3, &gas3Text);
  gas4Text.attachPop(showThresholds4, &gas4Text);
  gas1ValueText.attachPop(showThresholds1, &gas1ValueText);
  gas2ValueText.attachPop(showThresholds2, &gas2ValueText);
  gas3ValueText.attachPop(showThresholds3, &gas3ValueText);
  gas4ValueText.attachPop(showThresholds4, &gas4ValueText);
  unit1Text.attachPop(showThresholds1, &unit1Text);
  unit2Text.attachPop(showThresholds2, &unit2Text);
  unit3Text.attachPop(showThresholds3, &unit3Text);
  unit4Text.attachPop(showThresholds4, &unit4Text);
  b1Button.attachPop(showThresholds1, &b1Button);
  b2Button.attachPop(showThresholds2, &b2Button);
  b3Button.attachPop(showThresholds3, &b3Button);
  b4Button.attachPop(showThresholds4, &b4Button);
  backButton1.attachPop(printData, &backButton1);
  backButton2.attachPop(printData, &backButton2);
  backButton3.attachPop(printData, &backButton3);
  backButton4.attachPop(printData, &backButton4);

  while (!Serial);
  //Serial.println("LoRa Receiver");
  LoRa.setPins(10, 9, 2);
  //LoRa.setSPIFrequency(868E6);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.enableCrc();

}

void loop() {
  loraReceive();
  //This function will check if an alarmFlag is true and will start the alarm if so
  alarm();
  //An empty LoRa packet is sent to the instructor device, for signal strength
  sendData();
  checkButtons();
  batteryMeasurement();
  nexLoop(nex_listen_list);
}

void checkButtons(){
  //Button pressed: turn off alarms
  if(digitalRead(ACKBUT) == HIGH){
    alarmFlag1 = false;
    alarmFlag2 = false;
  }

  if(digitalRead(TOGGLEBUT) == HIGH && toggleDebounce == false){
    toggleDebounce = true;
    if(page == 4){
      page = 0;
    }
    else{
      page++;
    }
    sprintf(val, "page %i", page);
    Serial.print(val);
    serialEnd();
    updatePage();
  }

  else if(digitalRead(TOGGLEBUT) == LOW && toggleDebounce == true){
    toggleDebounce = false;
  }
}

void loraReceive(){
  if(LoRa.parsePacket()){
    while(LoRa.available()){
      if(LoRa.read() == localAddress){
        byte mode = LoRa.read();
        if(mode == 0xFF){  //GAS VALUES
          for(int i = 0; i < 4; i++){
            gas[i] = word(LoRa.read(), LoRa.read());
            gasPoint[i] = word(LoRa.read(), LoRa.read());
          }
          LoRa.beginPacket();
          LoRa.write(destination);
          LoRa.write(0xAA);
          LoRa.endPacket();
          checkGasses();
        }

        else if(mode == 0x00){ //THRESHOLD VALUES
          for(int i = 0; i < 8; i++){
            tres[i] = word(LoRa.read(), LoRa.read());
            tresPoint[i] = word(LoRa.read(), LoRa.read());
          }
          LoRa.beginPacket();
          LoRa.write(destination);
          LoRa.write(0xAA);
          LoRa.endPacket();
        }
        updatePage();
      }
    }
  }
}

int compareGas(int index){
  if(gasPoint[index] == SET){
    return gas[index];
  }
  else{
    return gas[index]*10;
  }
}

int compareTres(int index){
  if(tresPoint[index] == SET){
    return tres[index];
  }
  else{
    return tres[index]*10;
  }
}

void checkGasses(){
  for(int i = 0; i < 4; i++){
    switch (i) {
      case 1: //O2
        if(compareGas(i) >= compareTres(2*i+1)){
          alarmFlag2 = true;
          setAlarmBackground(i+1, 1);
        }
        else if(compareGas(i) <= compareTres(2*i)){
          alarmFlag1 = true;
          setAlarmBackground(i+1, 1);
        }
        else{
          setAlarmBackground(i+1, 0);
        }
        break;

      default: //All other gasses
        if(compareGas(i) >= compareTres(2*i+1)){
          alarmFlag2 = true;
          setAlarmBackground(i+1, 1);
        }
        else if(compareGas(i) >= compareTres(2*i)){
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

void batteryMeasurement() {
  if(millis() - lastBatteryMeasurement > 1000){
    float rawV = (analogRead(BATTERY) * 5.0) / 1024;      //figure out the battery voltage (4.98 is the actual reading of my 5V pin)                                              //some logic to set values

    int pic;
    if (rawV < 3.7) {                           //battery @ 3.5V or less
      pic = 6;
    }
    else if (rawV > 3.7 && rawV < 3.9) {               //battery @ 3.8V
      pic = 3;
    }
    else if (rawV > 3.9 && rawV < 4.1) {               //battery @ 3.9V
      pic = 5;
    }
    else if (rawV > 4.1) {                            //battery @ 4.2V 100% battery
      pic = 0;
    }
    sprintf(val, "battery.pic=%i", pic);
    Serial.print(val);
    serialEnd();
  }
}
