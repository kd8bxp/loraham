/* This is a beacon for the LoRaHam protocol by KK4VCZ.
 * https://github.com/travisgoodspeed/loraham/
 * 
 * Modified by LeRoy Miller, KD8BXP
 * Aug 25, 2017 For use with the TTGO WIFI_LoRa_32 Boards
 * https://www.aliexpress.com/item/TTGO-loraSX1278-ESP32-0-96OLED-16-Mt-bytes-128-Mt-bit-433Mhz-for-arduino/32824758955.html
 * 
 * Significant changes:
 * 
 * Library Changed to: arduino-LoRa
 * https://github.com/sandeepmistry/arduino-LoRa
 * 
 */

#define CALLSIGN "KD8BXP-10" 
#define COMMENTS "2600mAh ESP32" 
 
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

#define VBATPIN 36 //battery voltage check pin
#define LED 25 //LED Pin

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  300        /* Time ESP32 will go to sleep (in seconds) 5 minutes*/

RTC_DATA_ATTR int packetnum=0;

//Returns the battery voltage as a float.
float voltage(){

  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return measuredvbat;
}

void radioon(){
 
}

void radiooff(){
  //arduino-LoRa library provides a sleep, and a idle mode (Sleep is what I beleive we need here) 
  //I'm not sure this is needed once deep sleep was turned on. More
  //Information is needed.
  LoRa.sleep();
  delay(10);
}

void setup() 
{
  pinMode(OLED_RESET,OUTPUT);
  digitalWrite(OLED_RESET, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_RESET, HIGH); // while OLED is running, must set
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  Serial.setTimeout(10);
  while (!Serial); //if just the the basic function, must connect to a computer
  delay(1000);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
 /* display.setColor(WHITE);
  display.drawString(0,0,"Callsign: ");
  display.drawString(45, 0,CALLSIGN);
  display.drawString(35,11,"LoRaHam Beacon");
  display.drawString(47,22,"by KK4VCZ.");
  display.drawString(32,33,"modified for ESP32");
  display.drawString(50,44,"by KD8BXP.");
  display.display();
  delay(3000); //while delayed can't get messages
  display.clear();
  display.display();
*/

  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.enableCrc(); //encable CRC checking - off by default
  digitalWrite(LED, LOW);
  esp_deep_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}


//! Uptime in seconds, correcting for rollover. 
//Gives incorrect information since deep sleep installed. (always says
// zero or null)
  long int uptime(){
  static unsigned long rollover=0;
  static unsigned long lastmillis=millis();

  //Account for rollovers, every ~50 days or so.
  if(lastmillis>millis()){
    rollover+=(lastmillis>>10);
    lastmillis=millis();
  }

  return(rollover+(millis()>>10));
}

//Transmits one beacon and returns.
void beacon(){
  digitalWrite(LED, LOW);
  digitalWrite(LED, HIGH);
  float vcc=voltage();
  //Serial.println("Transmitting..."); // Send a message to rf95_server
  char radiopacket[128];
  snprintf(radiopacket,
           128,
           "BEACON %s %s VCC=%d.%03d count=%d uptime=%ld",
           CALLSIGN,
           COMMENTS,
           (int) vcc,
           (int) (vcc*1000)%1000,
           packetnum,
           uptime());

  radiopacket[sizeof(radiopacket)] = 0;
  
  //Serial.println("Sending..."); delay(10);
  LoRa.beginPacket();
  LoRa.print(radiopacket);
  LoRa.endPacket();
  display.clear();
  display.setColor(WHITE);
  display.drawStringMaxWidth(0,0,110, radiopacket);
  display.println("");
  display.display();
  delay(1000);
  display.clear();
  display.display();
  packetnum++;
  digitalWrite(LED, LOW);
}

void loop(){
  //Transmit a beacon once every five minutes.
  beacon();
  //Then turn the radio off to save power.
  radiooff();
  esp_deep_sleep_start();
}

