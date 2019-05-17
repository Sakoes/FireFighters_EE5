#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Nextion.h>
#include <EEPROM.h>

#define BATTERY A5


int signal_strength[5] = {0,0,0,0,0};

enum decimal {
  NO,
  CURRENT,
  SET
};


// union         //I can't find how to make an array of union maybe gas2[2]?
// {
//   float flat;
//   byte bytelat[4];
// } gps_lat;
//
// union
// {
//   float flon;
//   byte bytelon[4];
// } gps_lon;

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin

byte localAddress = 0xBC;     // address of this device
byte destination = 0xBB;      // destination to send to




//(page, id, objectName)
NexText gas1Text = NexText(0, 1, "gas1");
NexText gas2Text = NexText(0, 2, "gas2");
NexText gas3Text = NexText(0, 3, "gas3");
NexText gas4Text = NexText(0, 15, "gas4");
NexText gas1ValueText = NexText(0, 6, "gas1value");
NexText gas2ValueText = NexText(0, 7, "gas2value");
NexText gas3ValueText = NexText(0, 8, "gas3value");
NexText gas4ValueText = NexText(0, 16, "gas4value");
NexButton sendButton = NexButton(0, 9, "send");

NexButton settingsOkButton = NexButton(3, 3, "b1");
NexButton editTresButton = NexButton(3, 2, "editTres");

NexText tres1Text = NexText(4, 2, "tres1");
NexText tres2Text = NexText(4, 3, "tres2");
NexText tres3Text = NexText(4, 4, "tres3");
NexText tres4Text = NexText(4, 5, "tres4");
NexButton tresholdButton = NexButton(4, 6, "sendTres");
NexButton cancelTresButton = NexButton(4, 8, "cancel");

NexText a1Text = NexText(5, 4, "a1");
NexText a2Text = NexText(5, 5, "a2");

NexButton okButton = NexButton(1, 12, "ok");
NexButton cancelButton = NexButton(1, 14, "cancel");
NexButton dotButton = NexButton(1, 15, "dot");
NexButton backButton = NexButton(1, 13, "back");
NexButton zeroButton = NexButton(1, 1, "b0");
NexButton oneButton = NexButton(1, 2, "b1");
NexButton twoButton = NexButton(1, 3, "b2");
NexButton threeButton = NexButton(1, 4, "b3");
NexButton fourButton = NexButton(1, 5, "b4");
NexButton fiveButton = NexButton(1, 6, "b5");
NexButton sixButton = NexButton(1, 7, "b6");
NexButton sevenButton = NexButton(1, 8, "b7");
NexButton eightButton = NexButton(1, 9, "b8");
NexButton nineButton = NexButton(1, 10, "b9");


char val[60] = {0};



//These are the values of the gasses. If floats are needed, they are reformatted in the UI to show a decimal point
int gas[4];
int gasPrev[4];
decimal gasPoint[4];
decimal gasPointPrev[4];
int currentGas = 0;


//Values of thresholds
int tres[8] = {10,20,19,23,20,100,100,200};
int tresPrev[8];
decimal tresPoint[8] = {NO, NO, NO, NO, NO, NO, NO, NO};
decimal tresPointPrev[8] = {NO, NO, NO, NO, NO, NO, NO, NO};
int currentTres = 0;

int tresBackup[8];
decimal tresPointBackup[8];

bool message = false;
unsigned int lastMessage;

unsigned int lastBatteryMeasurement;


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
  &settingsOkButton,
  &editTresButton,
  &tres1Text,
  &tres2Text,
  &tres3Text,
  &tres4Text,
  &tresholdButton,
  &cancelTresButton,
  &a1Text,
  &a2Text,
  &sendButton,
  &okButton,
  &cancelButton,
  &dotButton,
  &backButton,
  &zeroButton,
  &oneButton,
  &twoButton,
  &threeButton,
  &fourButton,
  &fiveButton,
  &sixButton,
  &sevenButton,
  &eightButton,
  &nineButton,
  NULL
};  // End of touch event list



void serialEnd() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}




void gas1TextPopCallback(void *ptr) {
  currentGas = 1;
  Serial.print(F("gasText.txt=\"CH4\""));
  serialEnd();
  updateValue();
}

void gas2TextPopCallback(void *ptr) {
  currentGas = 2;
  Serial.print(F("gasText.txt=\"O2\""));
  serialEnd();
  updateValue();
}

void gas3TextPopCallback(void *ptr) {
  currentGas = 3;
  Serial.print(F("gasText.txt=\"CO\""));
  serialEnd();
  updateValue();
}

void gas4TextPopCallback(void *ptr) {
  currentGas = 4;
  Serial.print(F("gasText.txt=\"IBUT\""));
  serialEnd();
  updateValue();
}

void sendButtonPopCallback(void *ptr) {
  Serial.print(F("g0.txt=\"Sent executed\""));
  serialEnd();
  sendData();
}



//THRESHOLD FUNCTIONS

void tres1TextPopCallback(void *ptr){
  currentTres = 1;
  Serial.print(F("tresText.txt=\"CH4\""));
  serialEnd();
  updateTres();
}

void tres2TextPopCallback(void *ptr){
  currentTres = 3;
  Serial.print(F("tresText.txt=\"O2\""));
  serialEnd();
  updateTres();
}

void tres3TextPopCallback(void *ptr){
  currentTres = 5;
  Serial.print(F("tresText.txt=\"CO\""));
  serialEnd();
  updateTres();
}

void tres4TextPopCallback(void *ptr){
  currentTres = 7;
  Serial.print(F("tresText.txt=\"IBUT\""));
  serialEnd();
  updateTres();
}

void a1TextPopCallback(void *ptr){
  if(currentTres == 2 || currentTres == 4 || currentTres == 6 || currentTres == 8){
    currentTres--;
  }
  updateValue();
}

void a2TextPopCallback(void *ptr){
  if(currentTres == 1 || currentTres == 3 || currentTres == 5 || currentTres == 7){
    currentTres++;
  }
  updateValue();
}

void updateTres(){
  if(tresPoint[currentTres-1] == SET) {
    sprintf(val, "a1.txt=\"%i,%i\"", tres[currentTres-1] / 10, tres[currentTres-1] - (tres[currentTres-1] / 10) * 10);
    Serial.print(val);
    serialEnd();
  }
  else if(tresPoint[currentTres-1] == CURRENT) {
    sprintf(val, "a1.txt=\"%i,\"", tres[currentTres-1]);
    Serial.print(val);
    serialEnd();
  }
  else{
    sprintf(val, "a1.txt=\"%i\"", tres[currentTres-1]);
    Serial.print(val);
    serialEnd();
  }

  if(tresPoint[currentTres] == SET) {
    sprintf(val, "a2.txt=\"%i,%i\"", tres[currentTres] / 10, tres[currentTres] - (tres[currentTres] / 10) * 10);
    Serial.print(val);
    serialEnd();
  }
  else if(tresPoint[currentTres] == CURRENT) {
    sprintf(val, "a2.txt=\"%i,\"", tres[currentTres]);
    Serial.print(val);
    serialEnd();
  }
  else{
    sprintf(val, "a2.txt=\"%i\"", tres[currentTres]);
    Serial.print(val);
    serialEnd();
  }
}

void backupTres(void *ptr){
  for (int i = 0; i < 8; i++) {
    tresBackup[i] = tres[i];
  }
  for (int i = 0; i < 8; i++) {
    tresPointBackup[i] = tresPoint[i];
  }
}

void loadTres(void *ptr){
  for (int i = 0; i < 8; i++) {
    tres[i] = tresBackup[i];
  }
  for (int i = 0; i < 8; i++) {
    tresPoint[i] = tresPointBackup[i];
  }
}
// END OF THRESHOLD FUNCTIONS




void okButtonPopCallback(void *ptr) {
  if(currentGas != 0){ //Gas value changed
    Serial.print(F("page 0"));
    serialEnd();
    for (int i = 0; i < 4; i++) {
      gasPrev[i] = gas[i];
    }
    for (int i = 0; i < 4; i++) {
      gasPointPrev[i] = gasPoint[i];
    }
    //Save the values in EEPROM
    for(int i = 0; i < 4; i++){
      EEPROM.write(4*i, lowByte(gas[i]));
      EEPROM.write(4*i+1, highByte(gas[i]));
      EEPROM.write(4*i+2, lowByte(gasPoint[i]));
      EEPROM.write(4*i+3, highByte(gasPoint[i]));
    }

    updateHome();
  }

  else{ //Threshold value changed
    Serial.print(F("page 5"));
    serialEnd();
    for (int i = 0; i < 8; i++) {
      tresPrev[i] = tres[i];
    }
    for (int i = 0; i < 8; i++) {
      tresPointPrev[i] = tresPoint[i];
    }
    if(currentTres%2==0){
      currentTres--;
    }
    updateTres();
  }
}

void cancelButtonPopCallback(void *ptr) {
  if(currentGas != 0){ //Gas value changed
    Serial.print(F("page 0"));
    serialEnd();
    for (int i = 0; i < 4; i++) {
      gas[i] = gasPrev[i];
    }
    for (int i = 0; i < 4; i++) {
      gasPoint[i] = gasPrev[i];
    }
    updateHome();
  }

  else{ //Threshold value changed
    Serial.print(F("page 5"));
    serialEnd();
    for (int i = 0; i < 8; i++) {
      tres[i] = tresPrev[i];
    }
    for (int i = 0; i < 8; i++) {
      tresPoint[i] = tresPointPrev[i];
    }
    if(currentTres%2==0){
      currentTres--;
    }
    updateTres();
  }
}

void dotButtonPushCallback(void *ptr) {
  if(currentGas != 0){ //Editing gas value
    if(gasPoint[currentGas - 1] != NO){
      throwDecimalSetError();
    }
    else{
      gasPoint[currentGas - 1] = CURRENT;
    }
  }

  else{ //Editing threshold
    if(tresPoint[currentTres - 1] != NO){
      throwDecimalSetError();
    }
    else{
      tresPoint[currentTres - 1] = CURRENT;
    }
  }
  updateValue();
}

void backButtonPushCallback(void *ptr) {
  if(currentGas != 0){  //Editing gas value
    if(gasPoint[currentGas - 1] == SET){
      gasPoint[currentGas - 1] = CURRENT;
      gas[currentGas - 1] /= 10;
    }
    else if (gasPoint[currentGas - 1] == CURRENT){
      gasPoint[currentGas - 1] = NO;
    }
    else {
      gas[currentGas - 1] /= 10;
    }
  }
  else{ //Editing threshold
    if(tresPoint[currentTres - 1] == SET){
      tresPoint[currentTres - 1] = CURRENT;
      tres[currentTres - 1] /= 10;
    }
    else if (tresPoint[currentTres - 1] == CURRENT){
      tresPoint[currentTres - 1] = NO;
    }
    else {
      tres[currentTres - 1] /= 10;
    }
  }
  updateValue();
}

void numberPushed(int x) {
  if(currentGas != 0){ //Editing gas value
    int prev = gas[currentGas-1];
    decimal prevPoint = gasPoint[currentGas-1];
    if (gasPoint[currentGas - 1] == SET) {
      throwDecimalSetError();
    }
    else {
      if (gasPoint[currentGas - 1] == CURRENT) {
        gasPoint[currentGas - 1] = SET;
      }
      gas[currentGas - 1] = gas[currentGas - 1]*10 + x;


      if(currentGas < 3){ //For CH4 and O2 CHECK MAXIMUM
        if(gasPoint[currentGas-1] == SET && gas[currentGas-1] > 100*10){
          gasPoint[currentGas-1] = prevPoint;
          gas[currentGas-1] = prev;
          throwMaximumExceededError();
        }
        else if(gas[currentGas-1] > 100){
          gas[currentGas-1] = prev;
          throwMaximumExceededError();
        }
      }
      else if(currentGas == 3){ //For CO CHECK MAXIMUM
        if((gasPoint[currentGas-1] == SET && gas[currentGas-1] > 500*10) || gas[currentGas-1] < 0){
          gasPoint[currentGas-1] = prevPoint;
          gas[currentGas-1] = prev;
          throwMaximumExceededError();
        }
        else if(gas[currentGas-1] > 500){
          gas[currentGas-1] = prev;
          throwMaximumExceededError();
        }
      }
      else{ //For IBUT CHECK MAXIMUM
        if((gasPoint[currentGas-1] == SET && gas[currentGas-1] > 2000*10) || gas[currentGas-1] < 0){
          gasPoint[currentGas-1] = prevPoint;
          gas[currentGas-1] = prev;
          throwMaximumExceededError();
        }
        else if(gas[currentGas-1] > 2000){
          gas[currentGas-1] = prev;
          throwMaximumExceededError();
        }
      }
    }
  }
  else{
    if (tresPoint[currentTres - 1] == SET) {
      throwDecimalSetError();
    }
    else {
      if (tresPoint[currentTres - 1] == CURRENT) {
        tresPoint[currentTres - 1] = SET;
      }
      tres[currentTres - 1] *= 10;
      tres[currentTres - 1] += x;
    }
  }
  updateValue();
}

void zeroButtonPushCallback(void *ptr) {
  numberPushed(0);
}

void oneButtonPushCallback(void *ptr) {
  numberPushed(1);
}

void twoButtonPushCallback(void *ptr) {
  numberPushed(2);
}

void threeButtonPushCallback(void *ptr) {
  numberPushed(3);
}

void fourButtonPushCallback(void *ptr) {
  numberPushed(4);
}

void fiveButtonPushCallback(void *ptr) {
  numberPushed(5);
}

void sixButtonPushCallback(void *ptr) {
  numberPushed(6);
}

void sevenButtonPushCallback(void *ptr) {
  numberPushed(7);
}

void eightButtonPushCallback(void *ptr) {
  numberPushed(8);
}

void nineButtonPushCallback(void *ptr) {
  numberPushed(9);
}


void updateValue() {
  if(currentGas != 0){ //Editing gas value
    if(gasPoint[currentGas-1] == SET) {
      int t1 = gas[currentGas-1] / 10;
      int t2 = gas[currentGas-1] - (gas[currentGas-1] / 10) * 10;
      sprintf(val, "gasValue.txt=\"%i,%i\"", t1, t2);
      Serial.print(val);
      serialEnd();
    }
    else if(gasPoint[currentGas-1] == CURRENT) {
      sprintf(val, "gasValue.txt=\"%i,\"", gas[currentGas-1]);
      Serial.print(val);
      serialEnd();
    }
    else{
      sprintf(val, "gasValue.txt=\"%i\"", gas[currentGas-1]);
      Serial.print(val);
      serialEnd();
    }
  }

  else{ //Editing threshold value
    if(tresPoint[currentTres-1] == SET) {
      int t1 = tres[currentTres-1] / 10;
      int t2 = tres[currentTres-1] - (tres[currentTres-1] / 10) * 10;
      sprintf(val, "gasValue.txt=\"%i,%i\"", t1, t2);
      Serial.print(val);
      serialEnd();
    }
    else if(tresPoint[currentTres-1] == CURRENT) {
      sprintf(val, "gasValue.txt=\"%i,\"", tres[currentTres-1]);
      Serial.print(val);
      serialEnd();
    }
    else{
      sprintf(val, "gasValue.txt=\"%i\"", tres[currentTres-1]);
      Serial.print(val);
      serialEnd();
    }
  }
}

void updateHome() {
  for(int i = 0; i < 4; i++){
    if (gasPoint[i] == SET) {
      sprintf(val, "gas%ivalue.txt=\"%i,%i\"", i+1, gas[i] / 10, gas[i] - (gas[i] / 10) * 10);
      Serial.print(val);
      serialEnd();
    }
    else {
      sprintf(val, "gas%ivalue.txt=\"%i\"", i+1, gas[i]);
      Serial.print(val);
      serialEnd();
    }
  }
  currentGas = 0;
}

void throwDecimalSetError() {
  Serial.print(F("errorMessage.txt=\"Decimal is already set.\""));
  serialEnd();
}

void throwMaximumExceededError() {
  Serial.print(F("errorMessage.txt=\"Maximum value exceeded. Max CH4 and O2: 100. Max CO: 500. Max IBUT: 2000\""));
  serialEnd();
}

void sendTresholds(){
  int attempt = 0;
  bool ackReceived = false;

  while(!ackReceived && attempt < 10) {
    attempt++;
    LoRa.beginPacket();
    LoRa.write(destination);
    LoRa.write(0x00);
    for(int i = 0; i < 8; i++){
      LoRa.write(lowByte(tres[i]));
      LoRa.write(highByte(tres[i]));
      LoRa.write(lowByte(tresPoint[i]));
      LoRa.write(highByte(tresPoint[i]));
    }
    LoRa.endPacket();

    //check a few times whether data has been received, to give time to the receiver to send the package
    for(int i = 0; i < 10; i++){
      delay(5);
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        // received a packet

        // read packet
        while (LoRa.available()) {
          if (LoRa.read() == localAddress && LoRa.read() == 0xAA) {
            ackReceived = true;
            // bool dataIntact = true;
            // for(int i = 0; i < 8; i++){
            //   if(LoRa.read() != lowByte(tres[i]) || LoRa.read() != highByte(tres[i]) || LoRa.read() != lowByte(tresPoint[i]) || LoRa.read() != highByte(tresPoint[i])){
            //     dataIntact = false;
            //   }
            // }
            // if(dataIntact == true){
            //   ackReceived = true;
            // }
          }
        }
      }
    }
  }

  //Was an acknowledgement received?
  if(!ackReceived){
    Serial.print(F("message.txt=\"No acknowledgement received. Is the other device turned on and in range?\""));
    serialEnd();
    //Notify user that sending failed and that there might be connectivity issues
  }
  else{
    Serial.print(F("page 3"));
    serialEnd();
  }
  message = true;
  lastMessage = millis();
}

void sendData() {
  //The while loop will keep resending until an acknowledgement has been received
  //If no acknowledgement has been received after 10 attempts, stop the loop and notify the user
  int attempt = 0;
  bool ackReceived = false;

  while(!ackReceived && attempt < 10) {
    attempt++;
    LoRa.beginPacket();
    LoRa.write(destination);
    LoRa.write(0xFF);
    for(int i = 0; i < 4; i++){
      LoRa.write(lowByte(gas[i]));
      LoRa.write(highByte(gas[i]));
      LoRa.write(lowByte(gasPoint[i]));
      LoRa.write(highByte(gasPoint[i]));
    }
    LoRa.endPacket();

    //check a few times whether data has been received, to give time to the receiver to send the package
    for(int i = 0; i < 10; i++){
      delay(5);
      int packetSize = LoRa.parsePacket();
      if (packetSize){
        // received a packet

        // read packet
        while (LoRa.available()) {
          if (LoRa.read() == localAddress && LoRa.read() == 0xAA) {
            ackReceived = true;
            // bool dataIntact = true;
            // for(int i = 0; i < 4; i++){
            //   if(LoRa.read() != lowByte(gas[i]) || LoRa.read() != highByte(gas[i]) || LoRa.read() != lowByte(gasPoint[i]) || LoRa.read() != highByte(gasPoint[i])){
            //     dataIntact = false;
            //   }
            // }
            // if(dataIntact == true){
            //   ackReceived = true;
            // }
          }
        }
      }
    }
  }

  //Was an acknowledgement received?
  if(!ackReceived){
    Serial.print(F("message.txt=\"Transmission problems possible.\""));
    serialEnd();
    //Notify user that sending failed and that there might be connectivity issues
  }
  else{
    Serial.print(F("message.txt=\"Data sent!\""));
    serialEnd();
  }
  message = true;
  lastMessage = millis();
}


void signalStrength(){
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet

    // read packet
    while (LoRa.available()) {
      if (LoRa.read() == localAddress) {
        // for (int i = 0; i < 4; i++) {
        //   gps_lat.bytelat[i] = LoRa.read();
        // }
        // for (int i = 0; i < 4; i++) {
        //   gps_lon.bytelon[i] = LoRa.read();
        // }
        //Read GPS coordinates
        // for(int c = 0; c < 2; c++){ //c = 0: longitude, 1: latitude
        //   for(int i = 0; i < 4; i++){
        //     coordinates[c].bval[i] = LoRa.read();
        //   }
        // }
        //
        //Send values to display
        // char* num;
        // dtostrf(gps_lon.flon, 12, 7, num);
        // strcpy_P(val, (const char*) F("longitude.txt=\""));
        // strcat(val, num);
        // strcat_P(val, (const char*) F("\""));
        // Serial.print(val);
        // serialEnd();
        // dtostrf(gps_lat.flat, 12, 7, num);
        // strcpy_P(val, (const char*) F("latitude.txt=\""));
        // strcat(val, num);
        // strcat_P(val, (const char*) F("\""));
        // Serial.print(val);
        // serialEnd();

        //Calculate signal strength
        for(int i = 0; i < 4; i++){
          signal_strength[i] = signal_strength[i+1];
        }
        signal_strength[4] = LoRa.packetRssi();
        int average = 0;
        for(int i = 0; i < 5; i++){
          average += signal_strength[i];
        }
        average /= 5;
        int strength = average;
        sprintf(val, "signal.val=%i", strength);
        Serial.print(val);
        serialEnd();
        int pic;
        if(strength > -70){
          pic = 6;
        }
        else if(strength > -95){
          pic = 5;
        }
        else if(strength > -115){
          pic = 4;
        }
        else{
          pic = 3;
        }
        sprintf(val, "p0.pic=%i", pic);
        Serial.print(val);
        serialEnd();
        //LoRa.endPacket();
      }
      //Serial.print((char)LoRa.read());
    }
  }
}



void setup() {
  Serial.begin(9600);

  // put your setup code here, to run once:
  while (!Serial);

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(868E6)) {             // initialize ratio at 915 MHz
    while (true);                       // if failed, do nothing
  }
  LoRa.enableCrc();



  // put your setup code here, to run once:
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(A5, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, OUTPUT);
  nexInit();

  gas1Text.attachPop(gas1TextPopCallback, &gas1Text);
  gas2Text.attachPop(gas2TextPopCallback, &gas2Text);
  gas3Text.attachPop(gas3TextPopCallback, &gas3Text);
  gas4Text.attachPop(gas4TextPopCallback, &gas4Text);
  gas1ValueText.attachPop(gas1TextPopCallback, &gas1ValueText);
  gas2ValueText.attachPop(gas2TextPopCallback, &gas2ValueText);
  gas3ValueText.attachPop(gas3TextPopCallback, &gas3ValueText);
  gas4ValueText.attachPop(gas4TextPopCallback, &gas4ValueText);
  sendButton.attachPush(sendButtonPopCallback, &sendButton);

  settingsOkButton.attachPop(updateHome, &settingsOkButton);
  editTresButton.attachPop(backupTres, &editTresButton);

  tres1Text.attachPop(tres1TextPopCallback, &tres1Text);
  tres2Text.attachPop(tres2TextPopCallback, &tres2Text);
  tres3Text.attachPop(tres3TextPopCallback, &tres3Text);
  tres4Text.attachPop(tres4TextPopCallback, &tres4Text);
  tresholdButton.attachPop(sendTresholds, &tresholdButton);
  cancelTresButton.attachPop(loadTres, &cancelTresButton);

  a1Text.attachPop(a1TextPopCallback, &a1Text);
  a2Text.attachPop(a2TextPopCallback, &a2Text);

  okButton.attachPop(okButtonPopCallback, &okButton);
  cancelButton.attachPop(cancelButtonPopCallback, &cancelButton);
  dotButton.attachPush(dotButtonPushCallback, &dotButton);
  backButton.attachPush(backButtonPushCallback, &backButton);
  zeroButton.attachPush(zeroButtonPushCallback, &zeroButton);
  oneButton.attachPush(oneButtonPushCallback, &oneButton);
  twoButton.attachPush(twoButtonPushCallback, &twoButton);
  threeButton.attachPush(threeButtonPushCallback, &threeButton);
  fourButton.attachPush(fourButtonPushCallback, &fourButton);
  fiveButton.attachPush(fiveButtonPushCallback, &fiveButton);
  sixButton.attachPush(sixButtonPushCallback, &sixButton);
  sevenButton.attachPush(sevenButtonPushCallback, &sevenButton);
  eightButton.attachPush(eightButtonPushCallback, &eightButton);
  nineButton.attachPush(nineButtonPushCallback, &nineButton);


  //read gas values from EEPROM
  for(int i = 0; i < 4; i++){
    gas[i] = word(EEPROM.read(4*i+1), EEPROM.read(4*i));
    gasPoint[i] = word(EEPROM.read(4*i+3), EEPROM.read(4*i+2));
  }
  for (int i = 0; i < 4; i++) { //Copy current values to prev
    gasPrev[i] = gas[i];
  }
  for (int i = 0; i < 4; i++) { //Copy current values
    gasPointPrev[i] = gasPoint[i];
  }
  for (int i = 0; i < 8; i++) {
    tresPrev[i] = tres[i];
  }
  for (int i = 0; i < 8; i++) {
    tresPointPrev[i] = tresPoint[i];
  }
  updateHome();
}

void loop() {

  nexLoop(nex_listen_list);
  batteryMeasurement();
  signalStrength();

  checkMessage();
}

void checkMessage(){
  if(message == true && millis() - lastMessage > 8000){
    message = false;
    Serial.print(F("message.txt=\"\""));
    serialEnd();
  }
}

void batteryMeasurement() {
  if(millis() - lastBatteryMeasurement > 10000){
    float rawV = (analogRead(BATTERY) * 4.98) / 1024;      //figure out the battery voltage (4.98 is the actual reading of my 5V pin)                                              //some logic to set values
    int pic;
    if (rawV < 3.7) {                           //battery @ 3.5V or less
      pic = 10;
    }
    else if (rawV > 3.7 && rawV < 3.9) {               //battery @ 3.8V
      pic = 9;
    }
    else if (rawV > 3.9 && rawV < 4.1) {               //battery @ 3.9V
      pic = 8;
    }
    else if (rawV > 4.1) {                            //battery @ 4.2V 100% battery
      pic = 7;
    }
    sprintf(val, "battery.pic=%i", pic);
    Serial.print(val);
    serialEnd();
    lastBatteryMeasurement = millis();
  }

}
