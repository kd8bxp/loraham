void decodePacket(String pkt) {
  StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.parseObject(pkt);
Serial.println(pkt);
if (!root.success()) {
    Serial.println("parseObject() failed");
    
  }

 // Clear the buffer.
 String to = root["TO"];
 String from = root["FROM"];
 String msg1 = root["MESSAGE"];
String rt = root["RT"];
 TO = to;
 FROM = from;
 MSG1 = msg1;
 RT = rt;
}

