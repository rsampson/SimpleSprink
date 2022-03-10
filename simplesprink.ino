#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ezTime.h>
#include "password.h"   // keep your private keys and passwords in this format:
/*
   EXAMPLE:
  const char* ssid = "xxxx";
  const char* password = "xxxx";
  const char* APIKey = "xxxx";
  #define MYCHANNEL xxxx
*/


//  8 Relay board data at:
//  http://wiki.sunfounder.cc/index.php?title=8_Channel_5V_Relay_Module&utm_source=thenewstack&utm_medium=website

#define RELAY8
#ifdef  RELAY8     // if using a board with 8 relays
int relay[8] = {16,5,4,0,2,14,12,13};  // this is the output gpio pin ordering
#else              // else using board with 4 relays
int relay[4] = {16,14,12,13};
#endif



// Set watering schedule here
#define RUN_HOUR 7     // hour to start running
#define RUN_MINUTE 10    // minute to start running 
#define DURATION1 5     // how long to run staton 1, etc
#define DURATION2 5
#define DURATION3 5
#define DURATION4 7
#define DURATION5 7    
#define DURATION6 7
#define DURATION7 1
#define DURATION8 1

// do not modify these definitions
#define START1  RUN_MINUTE
#define START2  RUN_MINUTE + DURATION1 
#define START3  RUN_MINUTE + DURATION1 + DURATION2 
#define START4  RUN_MINUTE + DURATION1 + DURATION2 + DURATION3 
#define START5  RUN_MINUTE + DURATION1 + DURATION2 + DURATION3 + DURATION4
#define START6  RUN_MINUTE + DURATION1 + DURATION2 + DURATION3 + DURATION4 + DURATION5
#define START7  RUN_MINUTE + DURATION1 + DURATION2 + DURATION3 + DURATION4 + DURATION5 + DURATION6
#define START8  RUN_MINUTE + DURATION1 + DURATION2 + DURATION3 + DURATION4 + DURATION5 + DURATION6 + DURATION8

int pushButton = D2; // for reference only D1 and D2 can be used for unit # input
Timezone myTZ;


void setup_wifi() {

  // Connect to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// !!!!!!!!!!!!!!!!! It appears that the "on" state for the 8 relay board may be different from the 4 realy board , please check !!!!!!!!!!!!!
// turn relay rl on, all others off
void relayOn(int rl) {
  int i;
  for(i =0; i < sizeof relay / sizeof relay[0]; i++) {
    digitalWrite(relay[i], i == rl ? LOW: HIGH);
  }
}

void allOff() {
  relayOn(100);   // nonexistant relay # turns everything off
}

void relayConfig( ) {
  int i;
  for(i =0; i < sizeof relay / sizeof relay[0]; i++) {
    pinMode(relay[i], OUTPUT);
  }
}

void setup() {
  relayConfig();
  allOff();

  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for Serial port to connect. Needed for native USB port only
  }
  setup_wifi();

  // *** OTA stuff ***

  ArduinoOTA.setHostname("simplesprink");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.begin();
  // end OTA

  // ****** time stuff *******
  // Uncomment the line below to see what it does behind the scenes
  // setDebug(INFO);

  waitForSync();

  // Provide official timezone names
  // https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
  // Note that if you change the city in the above example it will still get the Berlin information 
  // from the cache and not execute the setLocation until you run someTZ.clearCache()
  if (!myTZ.setCache(0)) myTZ.setLocation(F("America/Los_Angeles"));
  myTZ.setDefault();
  Serial.println(myTZ.dateTime());
}


void loop() {

  ArduinoOTA.handle();
  events();

  if (weekday() == FRIDAY ||  weekday() == TUESDAY || weekday() == THURSDAY) {
    if (hour() == RUN_HOUR) {
      Serial.println("running");
      if     (minute() >= START1 && minute() < START2) {
        relayOn(0);
      }
      else if (minute() >= START2 && minute() < START3) {
        relayOn(1);
       }
      else if (minute() >= START3 && minute() < START4) {
        relayOn(2);
      }
      else if (minute() >= START4 && minute() < START5) {
        relayOn(3);
      }
#ifdef RELAY8     
      else if (minute() >= START5 && minute() < START6) {
        relayOn(4);
      }
      else if (minute() >= START6 && minute() < START7) {
        relayOn(5);
      }
      else if (minute() >= START7 && minute() < START8) {
        relayOn(6);
      }
      else if (minute() >= START8 && minute() < START8 + DURATION8) {
        relayOn(7);
      } 
#endif          
      else {
        allOff();
      }
    }
  }
}
