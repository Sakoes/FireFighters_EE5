#include <Nextion.h>
int i = 5;
NexButton okButton = NexButton(0, 5, "okButton"); //(page, id, objectName)
NexPage page0 = NexPage(0, 0, "page0");
int touched = 0;
String btnNames[5] = {"down.val=", "right.val=", "ok.val=", "left.val=", "up.val="};

NexTouch *nex_listen_list[] =
{
  &okButton,
  &page0,
  NULL
};  // End of touch event list

void okButtonPushCallback(void *ptr){
  digitalWrite(A1, HIGH);
  touched = 1;
}

void okButtonPopCallback(void *ptr){
  digitalWrite(A1, LOW);
  touched  = 0;
}

void setup() {
  int i = 5;


  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(A5, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, OUTPUT);

  Serial.begin(9600);
  nexInit();

  okButton.attachPush(okButtonPushCallback, &okButton);
  okButton.attachPop(okButtonPopCallback, &okButton);
}

void loop() {
  nexLoop(nex_listen_list);


}

void readBtnInputs(){
  bool inputStates[5];
  char displayCom [64];

  inputStates[0] = digitalRead(3);
  inputStates[1] = digitalRead(4);
  inputStates[2] = digitalRead(5);
  inputStates[3] = digitalRead(A0);
  inputStates[4] = digitalRead(A5);

  for(int i = 0; i < 5; i++){
    sprintf (displayCom, "%s%b", btnNames[i], inputStates[i]);
    Serial.print(displayCom);
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
  }
}
