/* LoRaHam Pager Example
 * 
 * Pager Sketch by KK4VCZ.
 * 
 * Modified by LeRoy Miller, KD8BXP
 * Aug 25, 2017 For use with the TTGO WIFI_LoRa_32 Boards
 * https://www.aliexpress.com/item/TTGO-loraSX1278-ESP32-0-96OLED-16-Mt-bytes-128-Mt-bit-433Mhz-for-arduino/32824758955.html
 * 
 * Significant changes:
 * LoRa Library Changed to: arduino-LoRa
 * https://github.com/sandeepmistry/arduino-LoRa
 * 
 * Addfruit_SSD1306 library changed to: squix78 esp8266 ssd1306 library
 * https://github.com/squix78/esp8266-oled-ssd1306
 * 
 * Attempt to Tokenized LoRaHam Protocol to use JSON
 * Library: https://github.com/bblanchon/ArduinoJson/
 * 
 * Sept 2, 2017 Added ESP32 Webserver 
 * Library Needed: http://www.fisch.lu/junk/ESP32-WebServer.zip (unofficial port of the ESP8266WebServer Library)
 * More Information can be found in the Arduino-ESP32 issue tracker on github
 * https://github.com/espressif/arduino-esp32/issues/425
 * 
 * Webserver default address (192.168.4.1) At this time I'm not sure how to change it.
 * The webserver is made for small screens (Phones/Tablets) but does work from a desktop.
 * root directory will show last message.
 * /msg allows you to send a message (not yet fully ready for use)
 * /msg2 is the API call 192.168.4.1/msg2?TO=KD8BXP&MSG=Some Message
 * /cfg setup and change configuration of your pager from a website (not yet ready)
 * 
 * Wish List/Ideas to improve:
 * 
 * 1) Add buzzer, Buzzer when a message arrives for your Callsign
 *    a different sound for BEACON messages (*DONE*)
 * 2) Add dip switches to turn on/off features.    
 *    one feature: ALL messages (BEACON) or CALLSIGN ONLY messages (in Webserver)
 * 3) WIFI website or api to send messages (in progress)   
 * 4) MAYBE a bluetooth keyboard to send messages (rejected)
 * 5) add button to review previous received message. (In webserver)
 */


/* Change this to your own callsign, or keep it as BEACON to view all
   BEACON frames.
 */
String CALLSIGN = ""; //USER Defined Special CallSign to receive and display
String MYCALL = "KD8BXP"; //display messages addressed to my call

#include <SPI.h>
#include <LoRa.h> //https://github.com/sandeepmistry/arduino-LoRa
#include <Wire.h>
#include "SSD1306.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESP32WebServer.h>

#define OLED_RESET  16  // Pin 16 -RESET digital signal
#define OLED_SDA    4  // SDA-PIN for I2C OLED
#define OLED_SCL    15  // SCL-PIN for I2C OLED

SSD1306 display(0x3c, OLED_SDA, OLED_SCL); // FOR I2C
ESP32WebServer server(80);

// WIFI_LoRa_32 ports

// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)

#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6 // Change to 434.0 or other frequency, must match RX's freq!

#define CHANNEL 0 //Used for Tone Generation
#define TONEPIN 32 //Pin USED for buzzer

// Blinky on receipt
#define LED 25

String TO, FROM, MSG1, RT;
char ssid[15]; //Create a Unique AP from MAC address
const char *password = "pass1234"; //not a great password, you may want to change it.
static int serverCore = 0; //run web server on this core. Loop() runs in core 1
int rssi=0;

void setup() {
  createSSID();
  pinMode(OLED_RESET,OUTPUT);
  digitalWrite(OLED_RESET, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_RESET, HIGH); // while OLED is running, must set GPIO16 in high
  pinMode(LED, OUTPUT);
  Serial.begin(9600);

ledcSetup(CHANNEL, 2000, 8);
ledcAttachPin(TONEPIN, CHANNEL);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.setColor(WHITE);
  display.drawString(0,0,"My Call: ");
  display.drawString(45, 0,MYCALL);
  display.drawString(35,11,"LoRaHam Pager");
  display.drawString(47,22,"by KK4VCZ.");
  display.drawString(32,33,"ESP32 Pager by");
  display.drawString(50,44,"KD8BXP.");
  display.display();
  delay(3000); //while delayed can't get messages
  display.clear();
  display.drawString(0,30,"SSID: ");
  display.drawString(30,30,ssid);
  display.display();
  delay(3000);
  display.clear();
  display.display();
  
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.enableCrc(); //encable CRC checking - off by default
  digitalWrite(LED, LOW);
   WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.print(myIP);
server.on("/", []()
      {
        String temp;
        temp = "<meta name=viewport content='width=200'><center><h1>LoRaHam</h1></center>Last Message:<br><br>To: " + TO +"<br>From: " +FROM+"<br>Message: "+MSG1+"<br>RT: "+RT+"<br><br><a href=\"/msg\">Send Message!</a>";
        Serial.println(temp);
        server.send(200, "text/html", temp);
      });
      
    server.on("/cfg", []()
      {
        server.send(200, "text/html", "CFG Screen");
      });

    server.on("/msg", []()
      {
        String form = "<meta name=viewport content='width=200'><style>input, textarea {max-width:100%}</style><p><center><form action='msg2'><p></center>TO: <input type='text' name='TO' size=15 vaule='"+FROM+"' autofocus><br>Message: <input type='text' name='MSG1' size=75><center><br><br> <input type='submit' value='Submit'></form></center>";
        server.send(200, "text/html", form);// And as regular external functions:
      });

    server.on("/msg2", []()
      {
        server.send(200,"text/html","Nothing Here Yet!");
      });
    server.begin();

    xTaskCreatePinnedToCore(
                    serverTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    serverCore);  /* Core where the task should run */
 
}

void loop() {
  // put your main code here, to run repeatedly:
  pager();
  vTaskDelay(10);
}
