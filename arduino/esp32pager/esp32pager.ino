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
 * Addfruit_SSD1306 library changed to: ESP_SSD1306 library which is based on the Adafruit library written for the ESP devices
 * https://github.com/somhi/ESP_SSD1306
 * 
 * Wish List/Ideas to improve:
 * 
 * 1) Add buzzer, Buzzer when a message arrives for your Callsign
 *    a different sound for BEACON messages
 * 2) Add dip switches to turn on/off features.    
 *    one feature: ALL messages (BEACON) or CALLSIGN ONLY messages
 * 3) WIFI website or api to send messages   
 * 4) MAYBE a bluetooth keyboard to send messages
 * 
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
   * // I don't believe this is needed for this board/library (?)
  Serial.println("Feather LoRa RX Test!");
  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  Serial.println("Set power to 23.");
  Serial.print("Max packet length: "); Serial.println(RH_RF95_MAX_MESSAGE_LEN);
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


  // text display tests
 // display.setTextSize(1);
  display.setColor(WHITE);
  //display.setCursor(0,0);
  display.drawString(0,0,(char*) pkt);
  //display.println("KK4VCZ de KD2JHL");
  //display.setColor(BLACK, WHITE); // 'inverted' text
  //display.println("The quick brown fox  jumps over the lazy  dog.");
  //display.setTextSize(2);
  //display.setColor(WHITE);
  //display.print("0x"); display.println(0xDEADBEEF, HEX);
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
  //display.setTextSize(1);
  display.setColor(WHITE);
  //display.setCursor(0,0);
  display.drawString(0,0,"Callsign: ");
  display.drawString(0,10,CALLSIGN);
  display.drawString(0,20,"LoRaHam Pager by\nKK4VCZ.");
  display.display();
  //display.clear();
  //displaypacket(NULL);
  delay(1000);

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
      //digitalWrite(LED, HIGH);
     }   
}

void loop() {
  // put your main code here, to run repeatedly:
  pager();
}
