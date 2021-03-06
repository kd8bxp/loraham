These are my own (KD8BXP) ideas for improvements/enhancements to the LoRaHam project. I also made the same comments on the master project at
https://github.com/travisgoodspeed/loraham/issues/17 - and can be tracked there.

Aug 26, 2017 -

Couple of thoughts I had while converting the pager.ino for use with a ESP32 board

Tokenize (sp) the sending string - making it easier to manipulate on the rx side.
IE: BEACON KD8BXP-10 VCC=3.742 Count=123 200mAh ESP32 would become something like this
BEACON;KD8BXP-10;VCC=3.742 Count=123 200mAh ESP32
And for RT it would become something like this:
BEACON;KD8BXP-10;VCC=3.742 Count=123 200mAh ESP32;RT KK4VCZ rssi=-46 The next RT would just add to the end without another ";" IE: BEACON;KD8BXP-10;VCC=3.742 Count=123 200mAh ESP32;RT KK4VCZ-16 rssi=-46 RT AB3XL-10 rssi=-80
This could be easy split, for display on a small OLED or other small screen.
To: BEACON From: KD8BXP-10 MSG: VCC=3.742 Count=123 200mAh ESP32 and on another screen
RT KK4VCZ-16 rssi=-46 RT AB3XL-10 rssi=-80 if there are RTs

My other thought would be to add some type of RT count, or the ability to know if "you" have already RT the packet. That being said, I have only looked at the code for BEACON and PAGER at this point. And if a filter is build into the gateway code then disregard this idea :-) (software could do this pretty easy)

You might considering adding something to the protocol that would tell a gateway to gate the message to the internet. And a way to track that it was gated, so it doesn't get gated a bunch of times. *I'm thinking maybe a semi-private MQTT server somewhere, the gateways could subscribe and publish to a "LoRaHam" topic for passing messages across the internet. I need to think about this idea a little more, and see how that fits into the plan. Using MQTT clients could be written that just use the internet to pass traffic. *

Some other ideas go into hardware design for the "pager" -

The board I have is a WIFI_LoRa_32 board, I found cheap on Aliexpress. It's is a ESP32, a 433Mhz SX1278 LoRa board, has a OLED (.96 I think), battery connector, and lots of GPIO. It's not without it's issues - Like radiohead library doesn't compile with it (hench, converting BEACON and PAGER to a different library), The Adafruit_SSD1306 doesn't work with it. (Someone converted that library for the ESP8266 and called it ESP_SSD1306 it's a drop in replacement for the Adafruit library) Unfortunately I couldn't get ESP_SSD1306 to work. And had to use a different SSD1306 library (link is in the sketch) But it was not a direct drop in replacement. Another issue with the board is some of the GPIO pins are covered by the OLED screen (IE: it would be hard to solder pins without damage to the screen - and something I need to look at closer)

Some of my ideas to add to the hardware/software:

Add a buzzer with a sound for "your callsign" and a different sound for the BEACON messages - this should be pretty easy to do. A Vibration motor would work just as well.
Add some DIP or other Switches - to turn on and off different features with out a need to reload a new sketch each time. This too should be pretty easy to do with a little thought as to what features you want to control.
A button or two to review the last message received - the button could also be used to move to the next screen when a message is too large to display on one screen. A little harder to do with out splitting the string (IE: my idea above)
Specific to the ESP32 hardware:
WIFI website/api to compose and send a short message from the pager. I think this will be fairly easy to do (?) Not sure how the ESP32 handles forms, so a API style maybe better (?)
Lastly
Using the ESP32 bluetooth and a bluetooth keyboard to compose and send a short message from the pager. I'm not even sure this can be done, it's just a thought.

--------
Aug 31, 2017 (More Ideas):
I would also like to propose a change from using BEACON to maybe SENS for SENSOR data being passed, and reserve BEACON to announce that there is a LoRaHam network in the area.
I would also like to propose adding the following:
CQ - For general calling and see who is out there - pagers would get the CQ
NET - used to start and respond to a NET
BLT (0-9) Bulletins used to make announcements to everyone
SPCL - for special event stations (I'm not sure if this would be used, but you never know)
WX - for weather stations - LoRa and Microcontrollers really lean themselfs to something like this. (I would also say these stations should be keep local, and not gated.)
SENS - Other type of sensor reading/RC control/etc - and should be keep local and not gated
CMD - a specific (different) protocol to command and control a remote BEACON or DIGI - should only be used local, not gated and only by the system operator (I haven't thought much more about this so a lot of thought would have to be put into something like this)

I would also suggest adding a general location to the protocol, that would help to keep packages that need to stay local - local. It wouldn't need to be precise maybe just the grid square like for me EM79 would work, even thou I do travel and sometime leave me "home" grid.

-----------
Attempt at a Change Log:

Aug 27, 2017 - Added Buzzer on Pin 32 - code as of right now just has a generic two tone sound, for any packet received.
Aug 31, 2017 - updated codes to use a JSON string.
Sep 1, 2017 Pager Code updated to display BEACON and MYCALL messages other messages will be RT. Buzzer Sounds are different for each type of message.
