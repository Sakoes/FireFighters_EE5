#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Nextion.h>

enum decimal {
  NO,
  CURRENT,
  SET
};


const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to



//(page, id, objectName)
NexPage page0 = NexPage(0, 0, "page0");
NexText gas1Text = NexText(0, 1, "gas1");
NexText gas2Text = NexText(0, 2, "gas2");
NexText gas3Text = NexText(0, 3, "gas3");
NexButton sendButton = NexButton(0, 10, "send");


NexPage page1 = NexPage(1, 0, "page1");
NexText errorMessageText = NexText(1, 17, "errorMessage");
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
int gas1 = 0;
int gas1prev;
decimal gas1point = NO;
int gas2 = 0;
int gas2prev;
decimal gas2point = NO;
int gas3 = 0;
int gas3prev;
decimal gas3point = NO;


NexTouch *nex_listen_list[] =
{
  &page0,
  &gas1Text,
  &gas2Text,
  &gas3Text,
  &sendButton,
  &page1,
  &errorMessageText,
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

void sendButtonPopCallback(void *ptr){
  int message = gas1;  // send a message
  sendMessage(message);
  Serial.println("Sending " + message);
  
}


void okButtonPopCallback(void *ptr) {
  gas1prev = gas1;
  gas2prev = gas2;
  gas3prev = gas3;
  currentGas = 0;
  updateHome();
}

void cancelButtonPopCallback(void *ptr) {
  gas1 = gas1prev;
  gas2 = gas2prev;
  gas3 = gas3prev;
  currentGas = 0;
  updateHome();
}

void dotButtonPushCallback(void *ptr) {
  if (currentGas == 1) {
    if (gas1point != NO) {
      throwDecimalSetError();
    }
    else {
      gas1point = CURRENT;
    }
  }
  else if (currentGas == 2) {
    if (gas2point != NO) {
      throwDecimalSetError();
    }
    else {
      gas2point = CURRENT;
    }
  }
  else {
    if (gas3point != NO) {
      throwDecimalSetError();
    }
    else {
      gas3point = CURRENT;
    }
  }
  updateValue();
}

void backButtonPushCallback(void *ptr) {
  if (currentGas == 1) {
    if (gas1point == SET) {
      gas1point = CURRENT;
      gas1 /= 10;
    }
    else if (gas1point == CURRENT) {
      gas1point = NO;
    }
    else{
      gas1 /= 10;
    }
  }
  else if (currentGas == 2) {
    if (gas2point == SET) {
      gas2point = CURRENT;
      gas2 /= 10;
    }
    else if (gas2point == CURRENT) {
      gas2point = NO;
    }
    else{
      gas2 /= 10;
    }
  }
  else {
    if (gas3point == SET) {
      gas3point = CURRENT;
      gas3 /= 10;
    }
    else if (gas3point == CURRENT) {
      gas3point = NO;
    }
    else{
      gas3 /= 10;
    }
  }
  updateValue();
}


void numberPushed(int x){
  if (currentGas == 1) {
    if (gas1point == SET) {
      throwDecimalSetError();
    }
    else {
      if (gas1point == CURRENT) {
        gas1point = SET;
      }
      gas1 *= 10;
      gas1 += x;
    }
  }
  else if (currentGas == 2) {
    if (gas2point == SET) {
      throwDecimalSetError();
    }
    else {
      if (gas2point == CURRENT) {
        gas2point = SET;
      }
      gas2 *= 10;
      gas2 += x;
    }
  }
  else {
    if (gas3point == SET) {
      throwDecimalSetError();
    }
    else {
      if (gas3point == CURRENT) {
        gas3point = SET;
      }
      gas3 *= 10;
      gas3 += x;
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
  if (currentGas == 1) {
    if (gas1point == SET) {
      int t1 = gas1 / 10;
      int t2 = gas1 - (gas1 / 10) * 10;
      sprintf(val, "gasValue.txt=\"%i,%i\"", t1, t2);
      Serial.print(val);
      serialEnd();
    }
    else if (gas1point == CURRENT) {
      sprintf(val, "gasValue.txt=\"%i,\"", gas1);
      Serial.print(val);
      serialEnd();
    }
    else {
      sprintf(val, "gasValue.txt=\"%i\"", gas1);
      Serial.print(val);
      serialEnd();
    }
  }
  else if (currentGas == 2) {
    if (gas2point == SET) {
      int t1 = gas2 / 10;
      int t2 = gas2 - (gas2 / 10) * 10;
      sprintf(val, "gasValue.txt=\"%i,%i\"", t1, t2);
      Serial.print(val);
      serialEnd();
    }
    else if (gas2point == CURRENT) {
      sprintf(val, "gasValue.txt=\"%i,\"", gas2);
      Serial.print(val);
      serialEnd();
    }
    else {
      sprintf(val, "gasValue.txt=\"%i\"", gas2);
      Serial.print(val);
      serialEnd();
    }
  }
  else {
    if (gas3point == SET) {
      int t1 = gas3 / 10;
      int t2 = gas3 - (gas3 / 10) * 10;
      sprintf(val, "gasValue.txt=\"%i,%i\"", t1, t2);
      Serial.print(val);
      serialEnd();
    }
    else if (gas3point == CURRENT) {
      sprintf(val, "gasValue.txt=\"%i,\"", gas3);
      Serial.print(val);
      serialEnd();
    }
    else {
      sprintf(val, "gasValue.txt=\"%i\"", gas3);
      Serial.print(val);
      serialEnd();
    }
  }
}

void updateHome(){
  if (gas1point == SET) {
    int t1 = gas1 / 10;
    int t2 = gas1 - (gas1 / 10) * 10;
    sprintf(val, "gas1value.txt=\"%i,%i\"", t1, t2);
    Serial.print(val);
    serialEnd();
  }
  else {
    sprintf(val, "gas1value.txt=\"%i\"", gas1);
    Serial.print(val);
    serialEnd();
  }
  
  if (gas2point == SET) {
    int t1 = gas2 / 10;
    int t2 = gas2 - (gas2 / 10) * 10;
    sprintf(val, "gas2value.txt=\"%i,%i\"", t1, t2);
    Serial.print(val);
    serialEnd();
  }
  else {
    sprintf(val, "gas2value.txt=\"%i\"", gas2);
    Serial.print(val);
    serialEnd();
  }
  
  if (gas3point == SET) {
    int t1 = gas3 / 10;
    int t2 = gas3 - (gas3 / 10) * 10;
    sprintf(val, "gas3value.txt=\"%i,%i\"", t1, t2);
    Serial.print(val);
    serialEnd();
  }
  else {
    sprintf(val, "gas3value.txt=\"%i\"", gas3);
    Serial.print(val);
    serialEnd();
  }
}

void serialEnd() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void throwDecimalSetError() {
  errorMessageText.setText("Decimal is already set.");
}

void sendMessage(int number) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  sprintf(val, "%i", number);
  String str(val);
  LoRa.write(str.length());        // add payload length
  LoRa.print(str);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    return;                             // skip rest of function
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

  Serial.println("LoRa init succeeded.");


  
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
  sendButton.attachPop(sendButtonPopCallback, &sendButton);


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
  onReceive(LoRa.parsePacket());
  // put your main code here, to run repeatedly:

}
