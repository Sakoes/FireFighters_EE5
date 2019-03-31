#include <SPI.h>
#include <LoRa.h>
#include <Nextion.h>


enum decimal {
  NO,
  CURRENT,
  SET
};

int gas[3];
decimal gasPoint[3];

char val[50] = {0};

byte localAddress = 0xBB;     // address of this device
byte destination = 0xBB;

void serialEnd() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}



void setup() {
  Serial.begin(9600);
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

void printData(){
  serialEnd();
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
          gas[0] = word(LoRa.read(), LoRa.read());
          gasPoint[0] = LoRa.read();
          gas[1] = word(LoRa.read(), LoRa.read());
          gasPoint[1] = LoRa.read();
          gas[2] = word(LoRa.read(), LoRa.read());
          gasPoint[2] = LoRa.read();
          //counter = LoRa.read();
          printData();

          //LoRa.endPacket();
      }
      //Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    sprintf(val, "lora.txt=\"%i\"", LoRa.packetRssi());
    Serial.print(val);
    serialEnd();
    //Serial.println(LoRa.packetRssi());
  }
}
