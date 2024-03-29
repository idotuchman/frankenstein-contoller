#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"  // see https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299

// hardware settings
#define SERIAL_RX_PIN D1      // input
#define SERIAL_TX_PIN D2      // output
#define RELAY_SPARK_PIN D3    // output
#define RELAY_STROBE_PIN D4   // output
#define MAIN_SWITCH_PIN D5    // input
#define DEBOUNCE_TIME_MS 20   // how long to check for noise on switchs

// scenes
#define ACT1_SCENE_TIME 6000
#define ACT2_SCENE_TIME 3000
#define ACT3_SCENE_TIME 2000
#define ACT4_SCENE_TIME 3000
#define ACT5_SCENE_TIME 2000
#define ACT6_SCENE_TIME 6000
enum statePerform {
  ACT1_START,   // main switch pulled, machine charging
  ACT1_SCENE,
  ACT2_START,   // monster thrashing (sparks and strobe 3 sec)
  ACT2_SCENE,
  ACT3_START,   // quiet for 2 sec
  ACT3_SCENE,
  ACT4_START,   // monster thrashing (sparks and strobe 3 sec)
  ACT4_SCENE,
  ACT5_START,   // quiet for 2 sec
  ACT5_SCENE,
  ACT6_START,   // monster escapes (strobe on)
  ACT6_SCENE
};

// sound effects
#define SOUND_MACHINE_HUM 1
#define SOUND_CHARGING 2
#define SOUND_THUD 3

// states
enum stateMachine {
  STOPPED,
  IDLING,
  PERFORMING
};
stateMachine state = STOPPED;
statePerform performanceState = ACT1_START;

static unsigned long sceneTimer;
SoftwareSerial mySoftwareSerial(SERIAL_RX_PIN, SERIAL_TX_PIN); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
bool switchPosition(int pin, bool state);    // debounce switch
void performSequence();

void setup() {
  pinMode(MAIN_SWITCH_PIN, INPUT);
  pinMode(RELAY_SPARK_PIN, OUTPUT);
  pinMode(RELAY_STROBE_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
  digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off

  
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);  // turn wifi off

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  // Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(5);  //Set volume value. From 0 to 30
}

void loop() {
  
  switch (state) {
    case IDLING:
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
      if (switchPosition(MAIN_SWITCH_PIN, HIGH)) {
        state = PERFORMING;
        myDFPlayer.stop();
        performanceState = ACT1_START;
      }
      break;
    case PERFORMING:
      performSequence();
      break;
    case STOPPED:
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
      myDFPlayer.stop();
      myDFPlayer.volume(5);  //Set volume value. From 0 to 30
      myDFPlayer.loop(SOUND_MACHINE_HUM);
      state = IDLING;
      break;
  }
}

void performSequence() {
  switch (performanceState) {
    case ACT1_START:          // machine charging
      sceneTimer = millis();  // start scene timer
      myDFPlayer.volume(20);  // set volume value. From 0 to 30
      // myDFPlayer.play(SOUND_THUD);
      // delay(1500);
      myDFPlayer.play(SOUND_CHARGING);
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
      Serial.println(F("Act 1: charging..."));
      performanceState = ACT1_SCENE;
      break;
    case ACT1_SCENE:
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
      if (millis() - sceneTimer > ACT1_SCENE_TIME) {
        myDFPlayer.stop();
        performanceState = ACT2_START;
      }
      break;
    case ACT2_START:          // monster thrashing (sparks and strobe 3 sec)
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, HIGH);
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      Serial.println(F("Act 2: Spark and strobe"));
      performanceState = ACT2_SCENE;
      break;
    case ACT2_SCENE:
      digitalWrite(RELAY_SPARK_PIN, HIGH);   // sparks on
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      if (millis() - sceneTimer > ACT2_SCENE_TIME) {
        performanceState = ACT3_START;
      }
      break;
    case ACT3_START:          // quiet for 2 sec
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
      Serial.println(F("Act 3: Pause"));
      performanceState = ACT3_SCENE;
      break;
    case ACT3_SCENE:
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
      if (millis() - sceneTimer > ACT3_SCENE_TIME) {
        performanceState = ACT4_START;
      }
      break;
    case ACT4_START:          // monster thrashing (sparks and strobe 3 sec)
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, HIGH);   // sparks on
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      Serial.println(F("Act 4: Spark and strobe"));
      performanceState = ACT4_SCENE;
      break;
    case ACT4_SCENE:          
      digitalWrite(RELAY_SPARK_PIN, HIGH);   // sparks on
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      if (millis() - sceneTimer > ACT4_SCENE_TIME) {
        performanceState = ACT5_START;
      }
      break;
    case ACT5_START:          // quiet for 2 sec
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
      Serial.println(F("Act 5: Pause"));
      performanceState = ACT5_SCENE;
      break;
    case ACT5_SCENE:
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
      if (millis() - sceneTimer > ACT5_SCENE_TIME) {
        performanceState = ACT6_START;
      }
      break;
    case ACT6_START:          // monster escapes (strobe on)
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, LOW);   // sparks off
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      Serial.println(F("Act 6: Strobe on, waiting for switch off"));
      performanceState = ACT6_SCENE;
      break;
    case ACT6_SCENE:
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      if (millis() - sceneTimer > ACT6_SCENE_TIME) {
        if(switchPosition(MAIN_SWITCH_PIN, LOW)) {
          digitalWrite(RELAY_STROBE_PIN, LOW);  // strobe off
          state = STOPPED;
        }
      }
      break;
  }
}

// checks if switch is at correct state during debounce period of time
bool switchPosition(int pin, bool state) {
  bool debounceState = true;
  unsigned long debounceTimer = millis();

  // keep checking that switch is at correct state
  while(millis() - debounceTimer < DEBOUNCE_TIME_MS) {
    if (digitalRead(pin) != state) {
      debounceState = false;
    }
  }
  return debounceState;
}