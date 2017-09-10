void digipeat(){
  digitalWrite(LED, LOW);
  digitalWrite(LED, HIGH);
  //float vcc=voltage();
  //Serial.println("Transmitting..."); // Send a message to rf95_server
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["TO"] = TO;
  root["FROM"] = FROM;
  
  //char TEMP[128];
  //snprintf(TEMP,128,"%s VCC=%d.%03d count=%d uptime=%ld",COMMENTS,(int)vcc,(int)(vcc*1000)%1000, packetnum,uptime());
  root["MESSAGE"] = MSG1;
  root["RT"] = MYCALL + " rssi=" + (String)rssi;
  
  /*char radiopacket[128];
  snprintf(radiopacket,
           128,
           PROTOCOL,
           TOCALL,
           CALLSIGN,
           COMMENTS,
           (int) vcc,
           (int) (vcc*1000)%1000,
           packetnum,
           uptime());

  radiopacket[sizeof(radiopacket)] = 0;
  */
  //Serial.println("Sending..."); delay(10);
  LoRa.beginPacket();
  //LoRa.print(radiopacket);
  String radiopacket;
  root.printTo(radiopacket);
  LoRa.print(radiopacket);
  Serial.println(radiopacket);
  LoRa.endPacket();
  display.clear();
  display.setColor(WHITE);
  display.drawString(20, 15, "Sending...");
  display.display();
  delay(3000);
  display.clear();
  display.display();
//  packetnum++;
  digitalWrite(LED, LOW);
}

