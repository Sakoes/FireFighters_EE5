#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Nextion.h>

String btnNames[5] = {"down.val=", "right.val=", "ok.val=", "left.val=", "up.val="};
bool inputStates[5] = {0};


enum decimal {
  NO,
  CURRENT,
  SET
};


const int csPin = 10;         // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin

byte localAddress = 0xBB;     // address of this device
byte destination = 0xBB;      // destination to send to



//(page, id, objectName)
NexPage page0 = NexPage(0, 0, "page0");
NexText gas1Text = NexText(0, 1, "gas1");
NexText gas2Text = NexText(0, 2, "gas2");
NexText gas3Text = NexText(0, 3, "gas3");
NexText gas1ValueText = NexText(0, 7, "gas1value");
NexText gas2ValueText = NexText(0, 8, "gas2value");
NexText gas3ValueText = NexText(0, 9, "gas3value");
NexButton sendButton = NexButton(0, 10, "send");


NexPage page1 = NexPage(1, 0, "page1");
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




char val[50] = {0};

int currentGas = 0;

//These are the values of the gasses. If floats are needed, they are reformatted in the UI to show a decimal point
int gas[3];
int gasPrev[3];
decimal gasPoint[3];



NexTouch *nex_listen_list[] =
{
  &page0,
  &gas1Text,
  &gas2Text,
  &gas3Text,
  &gas1ValueText,
  &gas2ValueText,
  &gas3ValueText,
  &sendButton,
  &page1,
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

void readBtnInputs(){
  char displayCom [64];

  inputStates[0] = digitalRead(3);
  inputStates[1] = digitalRead(4);
  inputStates[2] = digitalRead(5);
  inputStates[3] = digitalRead(A0);
  inputStates[4] = digitalRead(A5);



//  for(int i = 0; i < 5; i++){
//    sprintf (displayCom, "%s%b", btnNames[i], inputStates[i]);
//    Serial.print(displayCom);
//    serialEnd();
//  }
}


void gas1TextPopCallback(void *ptr) {
  currentGas = 1;
  Serial.print(F("gasText.txt=\"Explosives\""));
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

void sendButtonPopCallback(void *ptr) {
  Serial.print(F("g0.txt=\"Sent executed\""));
  serialEnd();
  sendData();
}


void okButtonPopCallback(void *ptr) {
  for (int i = 0; i < 3; i++) {
    gasPrev[i] = gas[i];
  }
  updateHome();
}

void cancelButtonPopCallback(void *ptr) {
  for (int i = 0; i < 3; i++) {
    gas[i] = gasPrev[i];
  }
  updateHome();
}

void dotButtonPushCallback(void *ptr) {
  if (gasPoint[currentGas - 1] != NO) {
    throwDecimalSetError();
  }
  else {
    gasPoint[currentGas - 1] = CURRENT;
  }
  updateValue();
}

void backButtonPushCallback(void *ptr) {
  if (gasPoint[currentGas - 1] == SET) {
    gasPoint[currentGas - 1] = CURRENT;
    gas[currentGas - 1] /= 10;
  }
  else if (gasPoint[currentGas - 1] == CURRENT) {
    gasPoint[currentGas - 1] = NO;
  }
  else {
    gas[currentGas - 1] /= 10;
  }
  updateValue();
}


void numberPushed(int x) {
  if (gasPoint[currentGas - 1] == SET) {
    throwDecimalSetError();
  }
  else {
    if (gasPoint[currentGas - 1] == CURRENT) {
      gasPoint[currentGas - 1] = SET;
    }
    gas[currentGas - 1] *= 10;
    gas[currentGas - 1] += x;
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

void updateHome() {
  for(int i = 0; i < 3; i++){
    if (gasPoint[i] == SET) {
      int t1 = gas[i] / 10;
      int t2 = gas[i] - (gas[i] / 10) * 10;
      sprintf(val, "gas%ivalue.txt=\"%i,%i\"", i+1, t1, t2);
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

void sendData() {
  Serial.print(F("g0.txt=\"Sending executed and finished\""));
  serialEnd();
  LoRa.beginPacket();
  LoRa.write(destination);
  LoRa.write(lowByte(gas[0]));
  LoRa.write(highByte(gas[0]));
  LoRa.write(gasPoint[0]);
  LoRa.write(lowByte(gas[1]));
  LoRa.write(highByte(gas[1]));
  LoRa.write(gasPoint[1]);
  LoRa.write(lowByte(gas[2]));
  LoRa.write(highByte(gas[2]));
  LoRa.write(gasPoint[2]);
  LoRa.endPacket();
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
  gas1ValueText.attachPop(gas1TextPopCallback, &gas1ValueText);
  gas2ValueText.attachPop(gas2TextPopCallback, &gas2ValueText);
  gas3ValueText.attachPop(gas3TextPopCallback, &gas3ValueText);
  sendButton.attachPush(sendButtonPopCallback, &sendButton);
  testButton.attachPush(sendButtonPopCallback, &testButton);


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
}

void loop() {

  nexLoop(nex_listen_list);
  readBtnInputs();
  // put your main code here, to run repeatedly:

}
