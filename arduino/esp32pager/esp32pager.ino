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
 * Wish List/Ideas to improve:
 * 
 * 1) Add buzzer, Buzzer when a message arrives for your Callsign
 *    a different sound for BEACON messages
 * 2) Add dip switches to turn on/off features.    
 *    one feature: ALL messages (BEACON) or CALLSIGN ONLY messages
 * 3) WIFI website or api to send messages   
 * 4) MAYBE a bluetooth keyboard to send messages
 * 5) add button to review previous received message.
 */


/* Change this to your own callsign, or keep it as BEACON to view all
   BEACON frames.
 */
#define CALLSIGN "BEACON"


#include <SPI.h>
#include <LoRa.h> //https://github.com/sandeepmistry/arduino-LoRa

#include <Wire.h>

#include "SSD1306.h"

#define OLED_RESET  16  // Pin 16 -RESET digital signal
#define OLED_SDA    4  // SDA-PIN for I2C OLED
#define OLED_SCL    15  // SCL-PIN for I2C OLED

SSD1306 display(0x3c, OLED_SDA, OLED_SCL); // FOR I2C

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


// Blinky on receipt
#define LED 25

void radioon(){
  /*
   * I don't believe this is needed for this board/library (?)  
   */
}

void radiooff(){
  //arduino-LoRa library provides a sleep, and a idle mode (Sleep is what I beleive we need here) 
  // manual reset
  //digitalWrite(RFM95_RST, LOW);
  LoRa.sleep();
  delay(10);
}


void displaypacket(char *pkt, int len){
  pkt[len]=0;
    
  // Clear the buffer.
  display.clear();
  display.setColor(WHITE);
  display.drawStringMaxWidth(0,0,100,(char*) pkt);
  //display.drawString(0,0,(String)pkt.substring(0,16));
  //display.drawString(0,10,(String)pkt.substring(17,33));
  //display.drawString(0,20,(String)pkt.substring(34,50));
  //display.drawString(0,30,(String)pkt.substring(51,67));
  //display.drawString(0,40,(String)pkt.substring(68,84));
  display.display();
  delay(2000);
  display.clear();
}

void setup() {
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  pinMode(LED, OUTPUT);
  Serial.begin(9600);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.setColor(WHITE);
  display.drawString(0,0,"Callsign: ");
  display.drawString(45, 0,CALLSIGN);
  display.drawString(35,11,"LoRaHam Pager");
  display.drawString(47,22,"by KK4VCZ.");
  display.drawString(32,33,"ESP32 Pager by");
  display.drawString(50,44,"KD8BXP.");
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
}


//Handles retransmission of the packet.
bool shouldirt(char *buf, uint8_t len){
  //Don't RT any packet containing our own callsign.
  //if(strcasestr((char*) buf, CALLSIGN)){
    displaypacket(buf,len);
  //  return false;
  //}
  
  //No objections.  RT it!
  return true;
}

//If a packet is available, digipeat it.  Otherwise, wait.
void pager(){
  digitalWrite(LED, LOW);
  //Try to receive a reply.
  
    String buf1;
    uint8_t len;
    int rssi=0;
  char temp;
    /*
     * When we receive a packet, we repeat it after a random
     * delay if:
     * 1. It asks to be repeated.
     * 2. We've not yet received a different packet.
     * 3. We've waited a random amount of time.
     * 4. The first word is not RT.
     */
     int packetSize = LoRa.parsePacket();
     if (packetSize) {
      digitalWrite(LED, HIGH);
     
  while (LoRa.available()) {
    temp = (char)LoRa.read();
    buf1 += temp;
  }
  char buf[buf1.length()];
  buf1.toCharArray(buf, buf1.length());
        rssi=LoRa.packetRssi();
        len = buf1.length();
        displaypacket(buf, len);
           }   
}

void loop() {
  // put your main code here, to run repeatedly:
  pager();
}
