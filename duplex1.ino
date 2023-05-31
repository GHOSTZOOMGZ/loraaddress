/*
  LoRa Duplex communication

  Sends a message every half second, and polls continually
  for new incoming messages. Implements a one-byte addressing scheme,
  with 0xFF as the broadcast address.

  Uses readString() from Stream class to read payload. The Stream class'
  timeout may affect other functuons, like the radio's callback. For an

  created 28 April 2017
  by Tom Igoe
*/
#include <SPI.h>              // include libraries
#include <LoRa.h>

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xFF;     // address of this device
byte destination = 0xBB;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 5000;          // interval between sends

void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);

  Serial.println("Lora первое устройство (Дуплекс)");

  // override the default CS, reset, and IRQ pins (optional)

  if (!LoRa.begin(450E6)) {             // initialize ratio at XXX MHz
    Serial.println("Устройство не готово, проверьте распиновку и настройки");
    while (true);                       // if failed, do nothing
  }
  LoRa.setSpreadingFactor(7);
  Serial.println("Устройство готово");
}
void loop() {
  if (millis() - lastSendTime > interval) {

    String message = "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";   // send a message
    sendMessage(message);
    Serial.println();
    Serial.println("Отправлено: " + message);
    lastSendTime = millis();            // timestamp the message
    interval = 20000;
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
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
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

// if message is for this device, or broadcast, print details:
  Serial.println();
  Serial.println("От: 0x" + String(sender, HEX)+" К: 0x" + String(recipient, HEX));
  Serial.println("Сообщение: " + incoming + " Номер: " + String(incomingMsgId) + " Уровень: " + String(LoRa.packetRssi()) + " дБм");
}

