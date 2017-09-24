/* 4-Way Button:  Click, Double-Click, Press+Hold, and Press+Long-Hold Test Sketch

Button login by By Jeff Saltzman, Oct. 13, 2009

To keep a physical interface as simple as possible, this sketch demonstrates generating four output events from a single push-button.
1) Click:  rapid press and release
2) Double-Click:  two clicks in quick succession
3) Press and Hold:  holding the button down
4) Long Press and Hold:  holding the button for a long time 
*/

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID        "__WIFI_SSID__"
#define WLAN_PASS        "__WIFI_PASSOWRD__"
#define BUTTON_PIN       14
#define MQTT_SERVER      "192.168.0.99"
#define MQTT_SERVERPORT  1883
#define MQTT_USERNAME    ""
#define MQTT_KEY         ""

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

// Setup a feed for publishing.
Adafruit_MQTT_Publish pop_button = Adafruit_MQTT_Publish(&mqtt, "home/livingroom/statelessswitch");

//=================================================

void MQTT_connect();
void setup() {
   // Set button input pin
   pinMode(BUTTON_PIN, INPUT_PULLUP);
   digitalWrite(BUTTON_PIN, HIGH);

   Serial.begin(115200);
   delay(10);
 
   // Connect to WiFi access point.
   Serial.println(); Serial.println();
   Serial.print("Connecting to ");
   Serial.println(WLAN_SSID);
 
   WiFi.begin(WLAN_SSID, WLAN_PASS);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println();
 
   Serial.println("WiFi connected");
   Serial.println("IP address: "); Serial.println(WiFi.localIP());
}

void loop() {
   // Ensure the connection to the MQTT server is alive (this will make the first
   // connection and automatically reconnect when disconnected).
   MQTT_connect();
  
   // Get button event and act accordingly
   int b = checkButton();
   if (b == 1) triggerEvent(0);
   if (b == 2) triggerEvent(1);
   if (b == 3) triggerEvent(2);
   // if (b == 4) triggerEvent(3);
}

//=================================================
// Event to trigger

void triggerEvent(int type) {
  String eventName = "";
  if(type == 0){ eventName = "single"; }
  else if(type == 1){ eventName = "double"; }
  else if(type == 2){ eventName = "hold"; }
  
  Serial.println(type + " Event");
  String msg = "{\"event\":\"" + eventName + "\",\"eventValue\":" + type + "}";
  
  if (! pop_button.publish(msg.c_str())) {
      Serial.println(F("Failed"));
  } else {
      Serial.println(F("OK!"));
  }
}


//=================================================
//  MULTI-CLICK:  One Button, Multiple Events

// Button timing variables
int debounce = 20;          // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 250;            // max ms between clicks for a double click event
int holdTime = 1000;        // ms hold period: how long to wait for press+hold event
int longHoldTime = 3000;    // ms long hold period: how long to wait for press+hold event

// Button variables
boolean buttonVal = HIGH;   // value read from button
boolean buttonLast = HIGH;  // buffered value of the button's previous state
boolean DCwaiting = false;  // whether we're waiting for a double click (down)
boolean DConUp = false;     // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true;    // whether it's OK to do a single click
long downTime = -1;         // time the button was pressed down
long upTime = -1;           // time the button was released
boolean ignoreUp = false;   // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false;        // when held, whether to wait for the up event
boolean holdEventPast = false;    // whether or not the hold event happened already
boolean longHoldEventPast = false;// whether or not the long hold event happened already

int checkButton() {    
   int event = 0;
   buttonVal = digitalRead(BUTTON_PIN);
   // Button pressed down
   if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce)
   {
       downTime = millis();
       ignoreUp = false;
       waitForUp = false;
       singleOK = true;
       holdEventPast = false;
       longHoldEventPast = false;
       if ((millis()-upTime) < DCgap && DConUp == false && DCwaiting == true)  DConUp = true;
       else  DConUp = false;
       DCwaiting = false;
   }
   // Button released
   else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce)
   {        
       if (not ignoreUp)
       {
           upTime = millis();
           if (DConUp == false) DCwaiting = true;
           else
           {
               event = 2;
               DConUp = false;
               DCwaiting = false;
               singleOK = false;
           }
       }
   }
   // Test for normal click event: DCgap expired
   if ( buttonVal == HIGH && (millis()-upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true && event != 2)
   {
       event = 1;
       DCwaiting = false;
   }
   // Test for hold
   if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
       // Trigger "normal" hold
       if (not holdEventPast)
       {
           event = 3;
           waitForUp = true;
           ignoreUp = true;
           DConUp = false;
           DCwaiting = false;
           //downTime = millis();
           holdEventPast = true;
       }
       // Trigger "long" hold
       if ((millis() - downTime) >= longHoldTime)
       {
           if (not longHoldEventPast)
           {
               event = 4;
               longHoldEventPast = true;
           }
       }
   }
   buttonLast = buttonVal;
   return event;
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
    int8_t ret;
  
    // Stop if already connected.
    if (mqtt.connected()) {
      return;
    }
  
    Serial.print("Connecting to MQTT... ");
  
    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
         Serial.println(mqtt.connectErrorString(ret));
         Serial.println("Retrying MQTT connection in 5 seconds...");
         mqtt.disconnect();
         delay(5000);  // wait 5 seconds
         retries--;
         if (retries == 0) {
           // basically die and wait for WDT to reset me
           while (1);
         }
    }
    Serial.println("MQTT Connected!");
}
