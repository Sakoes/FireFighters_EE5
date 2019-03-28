#include <SPI.h>
#include <LoRa.h>

int a = 0;
enum decimal {
  NO,
  CURRENT,
  SET
};
int counter = 0;
int gas1 = 0;
decimal gas1point = NO;
int gas2 = 0;
decimal gas2point = NO;
int gas3 = 0;
decimal gas3point = NO;

byte localAddress = 0xBB;     // address of this device
byte destination = 0xFE;
void setup() {
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
{ Serial.println(gas1);
  Serial.println(gas1point);
  Serial.println(gas2);
  Serial.println(gas2point);
  Serial.println(gas3);
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
          gas1 = word(LoRa.read(), LoRa.read());
          gas1point = LoRa.read();
          gas2 = word(LoRa.read(), LoRa.read());
          gas2point = LoRa.read();
          gas3 = word(LoRa.read(), LoRa.read());
          gas3point = LoRa.read();
          //counter = LoRa.read();
          printData();

          //LoRa.endPacket();
      }
      //Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
