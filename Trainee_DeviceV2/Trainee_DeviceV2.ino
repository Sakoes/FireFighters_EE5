#include <SPI.h>
#include <LoRa.h>
#define LED1   A0
#define LED2   A1
#define LED3   A2
#define ACKBUT A3
#define BUZZER 5
enum decimal {
  NO,
  CURRENT,
  SET
};
int gasCon; int gasType; //gas info
int counter = 0;

int gas[3]={0,0,0};  // CH4 IBUT O2 CO
decimal gas1point = NO;
decimal gas2point = NO;
decimal gas3point = NO;

unsigned long startMillis; unsigned long currentMillis;
const unsigned long period = 500 ;  //the value is a number of milliseconds, ie 1 second

byte localAddress = 0xBB;     // address of this device
byte destination = 0xFE;

boolean  ackflag    =   false; boolean  oldSwitch  =   LOW;
boolean  newSwitch  =   LOW; // toogle switch
boolean  alarmFlag1 =   false;
boolean  alarmFlag2 =   false;

void setup() {
  initializePins();
  startMillis=millis();
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");
  LoRa.setPins(10, 9, 2);
  //LoRa.setSPIFrequency(8E6);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void printData()
{ Serial.println(gas[0]);
  Serial.println(gas1point);
  Serial.println(gas[1]);
  Serial.println(gas2point);
  Serial.println(gas[2]);
  Serial.println(gas3point);
  }
void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      if(LoRa.read() == localAddress){
          //LoRa.beginPacket();
          gas[0] = word(LoRa.read(), LoRa.read());
          gas1point = LoRa.read();
          gas[1] = word(LoRa.read(), LoRa.read());
          gas2point = LoRa.read();
          gas[2]= word(LoRa.read(), LoRa.read());
          gas3point = LoRa.read();
          //counter = LoRa.read();
          printData();
          toggleSwtich();
          if(ackflag==false){gasConcentration(0,gas[0]); // gasConcentration(gasType,gasConcentration)
                             if(alarmFlag1==false && alarmFlag2==false){gasConcentration(1,gas[1]);}
                             else if(alarmFlag1==false && alarmFlag2==false) {gasConcentration(2,gas[2]);}
                             alarm();
          }
          else{stopAlarm();}
          //LoRa.endPacket();
      }
      //Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
void gasConcentration(int gasType,int gasCon){   //info about gasType and gas concentration
  /*gasType Gas  : max   A1  A2  unit
        0   CH4  : 100   10  20  %
        1   IBUT : 2000  100 200 ppm
        2   O2   : 25    19  23  % lower than 19 higher than 23 is not okay
        3   CO   : 500   20  100 ppm
  */
     if(gasType==0)       {setAlarm(gasCon,10,20);
     gas[0]=gasCon;}
     else if(gasType==1)  { setAlarm(gasCon,100,200);
     gas[1]=gasCon;}
     else if(gasType==2)  { setAlarmO2(gasCon,19,23);
     gas[2]=gasCon;}
     /*else if(gasType==3)  {setAlarm(gasCon,20,100);
     amountOfGas[3]=gasCon;}*/
  }
void setAlarm( int gasCon, int A1, int A2){  //choose between alarm 1 and alarm 2 and set alarm flag
   if(gasCon>=A1 && gasCon<A2){
      alarmFlag2=false;
      alarmFlag1=true;}
    else if (gasCon>=A2){
        alarmFlag2=true;
        alarmFlag1=false;
        }
    else {
        alarmFlag2=false;
        alarmFlag1=false;
      }
}
void setAlarmO2( int gasCon, int A1, int A2){  //choose between alarm 1 and alarm 2 and set alarm flag
   if(gasCon<=A1){
      alarmFlag2=false;
      alarmFlag1=true;}
    else if (gasCon>=A2){
        alarmFlag2=true;
        alarmFlag1=false;
        }
    else {
        alarmFlag2=false;
        alarmFlag1=false;
      }
}
void alarm(){
  currentMillis = millis(); //get the current time (number of milliseconds since the program started)
  if(currentMillis-startMillis >= period)
  {
    if(alarmFlag1==true || alarmFlag2 ==true){
    digitalWrite(LED1,!digitalRead(LED1));
    digitalWrite(LED2,!digitalRead(LED2));
    digitalWrite(LED3,!digitalRead(LED3));
    if(alarmFlag1==true && alarmFlag2==false ){tone(BUZZER,261,250 );}
    if(alarmFlag2==true && alarmFlag1==false) {tone(BUZZER,440,250);}
    startMillis=currentMillis;
    }
    else {stopAlarm();}
  }
}
void stopAlarm(){
  noTone(BUZZER);
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,LOW);
  digitalWrite(LED3,LOW);
}

void toggleSwtich(){
  newSwitch=digitalRead(ACKBUT);
  if(newSwitch !=oldSwitch){
      if(newSwitch==HIGH){
          if(ackflag==false) {ackflag=true;}
          else               {ackflag=false;}
        }
    }
    oldSwitch=newSwitch;
}

void initializePins() {
  pinMode(LED1,  OUTPUT);
  pinMode(LED2,  OUTPUT);
  pinMode(LED3,  OUTPUT);
  pinMode(BUZZER,OUTPUT);
  pinMode(ACKBUT,INPUT);
}
