#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// pin assignments
#define SERIAL_RX_PIN D1      // input
#define SERIAL_TX_PIN D2      // output
#define RELAY_SPARK_PIN D3    // output
#define RELAY_STROBE_PIN D4   // output
#define MAIN_SWITCH_PIN D5    // input

// sound effects
#define SOUND_MACHINE_HUM 1
#define SOUND_CHARGING 2
#define SOUND_THUD 3

static unsigned long sceneTimer;

// states
enum stateMachine {
  STOPPED,
  IDLING,
  PERFORMING
};
stateMachine state = STOPPED;

enum statePerform {
  ACT1_START,   // main switch pulled, machine charging
  ACT1_SCENE,
  ACT2_START,   // monster thrashing (pop and strobe 3 sec)
  ACT2_SCENE,
  ACT3_START,   // quiet for 2 sec
  ACT3_SCENE,
  ACT4_START,   // monster thrashing (pop and strobe 3 sec)
  ACT4_SCENE,
  ACT5_START,   // quiet for 2 sec
  ACT5_SCENE,
  ACT6_START,   // moster escapes (strobe on)
  ACT6_SCENE
};
statePerform performanceState = ACT1_START;


SoftwareSerial mySoftwareSerial(SERIAL_RX_PIN, SERIAL_TX_PIN); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
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

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
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
      if (digitalRead(MAIN_SWITCH_PIN)) {
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

  // if (myDFPlayer.available()) {
  //   printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  // }
}

void performSequence() {
  switch (performanceState) {
    case ACT1_START:          // machine charging
      sceneTimer = millis();  // start scene timer
      myDFPlayer.volume(20);  // set volume value. From 0 to 30
      // myDFPlayer.play(SOUND_THUD);
      // delay(1500);
      myDFPlayer.play(SOUND_CHARGING);
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, LOW);
      Serial.println(F("Act 1: charging..."));
      performanceState = ACT1_SCENE;
      break;
    case ACT1_SCENE:
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, LOW);
      if (millis() - sceneTimer > 6000) {
        myDFPlayer.stop();
        performanceState = ACT2_START;
      }
      break;
    case ACT2_START:          // monster thrashing (pop and strobe 3 sec)
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, HIGH);
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      Serial.println(F("Act 2: Spark and strobe"));
      performanceState = ACT2_SCENE;
      break;
    case ACT2_SCENE:
      digitalWrite(RELAY_SPARK_PIN, HIGH);
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      if (millis() - sceneTimer > 3000) {
        performanceState = ACT3_START;
      }
      break;
    case ACT3_START:          // quiet for 2 sec
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, LOW);
      Serial.println(F("Act 3: Pause"));
      performanceState = ACT3_SCENE;
      break;
    case ACT3_SCENE:          
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, LOW);
      if (millis() - sceneTimer > 2000) {
        performanceState = ACT4_START;
      }
      break;
    case ACT4_START:          // monster thrashing (pop and strobe 3 sec)
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, HIGH);
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      Serial.println(F("Act 4: Spark and strobe"));
      performanceState = ACT4_SCENE;
      break;
    case ACT4_SCENE:          
      digitalWrite(RELAY_SPARK_PIN, HIGH);
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      if (millis() - sceneTimer > 3000) {
        performanceState = ACT5_START;
      }
      break;
    case ACT5_START:          // quiet for 2 sec
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, LOW);
      Serial.println(F("Act 5: Pause"));
      performanceState = ACT5_SCENE;
      break;
    case ACT5_SCENE:
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, LOW);
      if (millis() - sceneTimer > 2000) {
        performanceState = ACT6_START;
      }
      break;
    case ACT6_START:          // moster escapes (strobe on)
      sceneTimer = millis();  // start scene timer
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      Serial.println(F("Act 6: Strobe on, waiting for switch off"));
      performanceState = ACT6_SCENE;
      break;
    case ACT6_SCENE:
      digitalWrite(RELAY_SPARK_PIN, LOW);
      digitalWrite(RELAY_STROBE_PIN, HIGH);
      if (millis() - sceneTimer > 6000) {
        if(digitalRead(MAIN_SWITCH_PIN) == LOW) {
          delay(500);
          if(digitalRead(MAIN_SWITCH_PIN)==LOW) {            
            digitalWrite(RELAY_STROBE_PIN, LOW);
            state = STOPPED;
          }
        }
      }
      break;
  }
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}