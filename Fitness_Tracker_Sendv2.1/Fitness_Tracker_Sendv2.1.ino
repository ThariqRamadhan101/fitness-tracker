// OLED //
// Memanggil Library
#include <Wire.h>
#include "SSD1306.h"
// Inisiasi Variabel
SSD1306  display(0x3C, D2, D1); //Address set here 0x3C that I found in the scanner, and pins defined as D2 (SDA/Serial Data), and D5 (SCK/Serial Clock).
int chan = 1;

// THINGSPEAK //
// Memanggil Library
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
// Inisiasi Variabel WiFi
char ssid[] = "Redmi";
char pass[] = "susahbanget";
int status = WL_IDLE_STATUS;
WiFiClient  client;
// Insiasi Variabel ThingSpeak
unsigned long myChannelNumber = 889176;
const char * myWriteAPIKey = "910NQQIYTGBWPSBD";

// Inisiasi Waktu
int period = 5000;
unsigned long time_now = millis();

// Split Data //
// Memanggil library
#include "StringSplitter.h"
// Inisiasi data
char msg;
String data;

String temp = "";
String bpm = "";
String countStep = "";

void setup()   {
  // put your setup code here, to run once:

  // Inisiasi Oled
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);

  // Inisiasi ThingSpeak
  WiFi.begin(ssid, pass);
  ThingSpeak.begin(client);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  // Mengambil data serial
  msg = Serial.read();
  if (msg == '{') {
    data = "";
  }
  if ((msg > 32) && (msg < 127) && (msg != '{') && (msg != '}') ) {
    data += msg;
  }
  // melakukan parsing
  if (msg == '}') {
    StringSplitter *splitter = new StringSplitter(data, ',', 3);
    int itemCount = splitter->getItemCount();
    for (int i = 0; i < itemCount; i++) {
      String item = splitter->getItemAtIndex(i);
      switch (i + 1) {
        case 1 :
          temp = item;
          break;
        case 2 :
          bpm  = item;
          break;
        case 3 :
          countStep = item;
          break;
      }
    }
    Serial.flush();
  }

  // Menampilkan di OLED
  display.clear();
  switch (chan) {
    case 1 :
      display.setFont(ArialMT_Plain_24);
      display.drawString(display.getWidth() / 2, display.getHeight() / 2 + 5, temp);
      display.setFont(ArialMT_Plain_16);
      display.drawString(display.getWidth() / 2, display.getHeight() / 2 + 20, "celcius");
      break;
    case 2 :
      display.setFont(ArialMT_Plain_24);
      display.drawString(display.getWidth() / 2, display.getHeight() / 2 + 5, bpm);
      display.setFont(ArialMT_Plain_16);
      display.drawString(display.getWidth() / 2, display.getHeight() / 2 + 20, "bpm");
      break;
    case 3 :
      display.setFont(ArialMT_Plain_24);
      display.drawString(display.getWidth() / 2, display.getHeight() / 2 + 5, countStep);
      display.setFont(ArialMT_Plain_16);
      display.drawString(display.getWidth() / 2, display.getHeight() / 2 + 20, "step");
      break;
  }
  display.display();

  // Setiap 5 detik mengirim data ke thingspeak dan menukar tampilan channel

  while (millis() > time_now + period) {
    // Mengirim data ke thingspeak
    ThingSpeak.setField(1, temp);
    ThingSpeak.setField(2, bpm);
    ThingSpeak.setField(3, countStep);
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    // Menukar tampilan channel
    chan++;
    if (chan > 3) {
      chan = 1;
    }
    time_now = millis();
  }

  // Melakukan delay
  delay(1);
}
