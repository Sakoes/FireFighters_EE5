#define LED1   A0
#define LED2   A1
#define LED3   A2
#define ACKBUT A3
#define BUZZER 5
unsigned long startMillis; unsigned long currentMillis;
const unsigned long period = 500 ;  //the value is a number of milliseconds, ie 1 second
boolean  ackflag    =   false; boolean  oldSwitch  =   LOW; boolean  newSwitch  =   LOW; // toogle switch
boolean  alarmFlag1 =   false;
boolean  alarmFlag2 =   false;
int gasCon; int gasType; //gas info
int amountOfGas[4];  // CH4 IBUT O2 CO
void setup() {
  initializePins();
  startMillis=millis();
}

void loop() {
   // this info is given by the instructor execute(gasType,gasCon)
  toggleSwtich();
  if(ackflag==false){gasConcentration(2,250);
                      alarm();
  }
  else{stopAlarm();}
}

void gasConcentration(int gasType,int gasCon){   //info about gasType and gas concentration
  /*gasType Gas  : max   A1  A2  unit
        0   CH4  : 100   10  20  %
        1   IBUT : 2000  100 200 ppm
        2   O2   : 25    19  23  % lower than 19 higher than 23 is not okay
        3   CO   : 500   20  100 ppm
  */
     if(gasType==0)       {setAlarm(gasCon,10,20);
     amountOfGas[0]=gasCon;}
     else if(gasType==1)  { setAlarm(gasCon,100,200);
     amountOfGas[1]=gasCon;}
     else if(gasType==2)  { setAlarmO2(gasCon,19,23);
     amountOfGas[2]=gasCon;}
     else if(gasType==3)  {setAlarm(gasCon,20,100);
     amountOfGas[3]=gasCon;}
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
