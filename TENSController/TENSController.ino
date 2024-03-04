#define ARDUINOOSC_DEBUGLOG_ENABLE
#include <ArduinoOSCWiFi.h>
#include "WiFi.h"

#define DEFAULT_OFF // Device is off by default

// Remove unused buttons to remove the feature
#define POWER_BUTTON 19 // Power Button
#define UP_BUTTON 18    // Intensity Up (REQUIRED)
#define DOWN_BUTTON 5   // Intensity Down (REQUIRED)
#define TIME_BUTTON 14  // Time Setting
#define PUP_BUTTON 12   // Preset Up
#define PDOWN_BUTTON 27 // Preset Down

#define M_PRESS_TIME_FOR_C 8 // Number of presses to enter continuous timer

#define BUTTON_DEBOUNCE_DELAY 45 // Your devices button minimum wait time in ms for its debouncer

// Stop / Disconnect Relays
#define CHANNEL_A_RELAY 16
#define CHANNEL_B_RELAY 17

// Relays On Off State (some relays are active on low or high)
#define RELAY_ON_STATE HIGH
#define RELAY_OFF_STATE LOW

// Max Intensivty Level supported by your device
#define MAX_INTENSITY 30

// WiFi Connection
const char * ssid = "Radio Noise AX";
const char * password = "Radio Noise AX";

int Current_Level = 0;
int TENS_Level = 0;
bool Current_Active = false;
bool TENS_Active = false;
bool TENS_ESTOP = true;
int Last_Command = 0;
int ActiveCommand = 0;
bool Pause_Commands = false;

TaskHandle_t HW_Controls;
TaskHandle_t Input_Hander;

void setup() {
  // Setup Button Pins
  #ifdef POWER_BUTTON
  pinMode(POWER_BUTTON, OUTPUT_OPEN_DRAIN); // POWER
  digitalWrite(POWER_BUTTON, HIGH);
  #endif
  pinMode(DOWN_BUTTON, OUTPUT_OPEN_DRAIN); // DOWN
  digitalWrite(DOWN_BUTTON, HIGH);
  pinMode(UP_BUTTON, OUTPUT_OPEN_DRAIN); // UP
  digitalWrite(UP_BUTTON, HIGH);
  #ifdef TIME_BUTTON
  pinMode(TIME_BUTTON, OUTPUT_OPEN_DRAIN); // TIME
  digitalWrite(TIME_BUTTON, HIGH);
  #endif
  #ifdef PUP_BUTTON
  pinMode(PUP_BUTTON, OUTPUT_OPEN_DRAIN); // PRESET_UP
  digitalWrite(PUP_BUTTON, HIGH);
  #endif
  #ifdef PDOWN_BUTTON
  pinMode(PDOWN_BUTTON, OUTPUT_OPEN_DRAIN); // PRESET_DOWN
  digitalWrite(PDOWN_BUTTON, HIGH);
  #endif
  // Setup Output Relays
  pinMode(CHANNEL_A_RELAY, OUTPUT); // CHANNEL A
  digitalWrite(CHANNEL_A_RELAY, RELAY_OFF_STATE);
  pinMode(CHANNEL_B_RELAY, OUTPUT); // CHANNEL B
  digitalWrite(CHANNEL_B_RELAY, RELAY_OFF_STATE);

  Serial.begin(115200);
  Serial.println("Initializing...");

  checkWiFiConnection();

  OscWiFi.subscribe(9001, "/avatar/parameters/TENSLevel", TENS_Level);   // Avatar Parameter for Level
  OscWiFi.subscribe(9001, "/avatar/parameters/TENSActive", TENS_Active); // Avatar Parameter for Enable
  OscWiFi.subscribe(9001, "/avatar/parameters/TENSESTOP", TENS_ESTOP);   // Avatar Parameter for E-Stop Toggle
  OscWiFi.subscribe(9001, "/avatar/parameters/KACommand", ActiveCommand);   // Avatar Parameter for Command

  delay(5000);
  #ifdef POWER_BUTTON
  #ifdef DEFAULT_OFF
  Serial.print("Booting TENS...");
  digitalWrite(POWER_BUTTON, HIGH);
  digitalWrite(POWER_BUTTON, LOW);
  delay(3000);
  digitalWrite(POWER_BUTTON, HIGH);
  delay(3000);
  #ifdef M_PRESS_TIME_FOR_C
  for (int i = 0; i < M_PRESS_TIME_FOR_C; i++) {
    digitalWrite(TIME_BUTTON, HIGH);
    digitalWrite(TIME_BUTTON, LOW);
    delay(BUTTON_DEBOUNCE_DELAY);
    digitalWrite(TIME_BUTTON, HIGH);
    delay(BUTTON_DEBOUNCE_DELAY);
  }
  #endif
  Serial.println(" OK");
  delay(4000);
  #endif
  #endif

  xTaskCreatePinnedToCore(
                  outputLoop,   /* Task function. */
                  "Hardware Controller",     /* name of task. */
                  10000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  1,           /* priority of the task */
                  &HW_Controls,      /* Task handle to keep track of created task */
                  0);          /* pin task to core 1 */

  xTaskCreatePinnedToCore(
                  inputLoop,   /* Task function. */
                  "Input Handeler",     /* name of task. */
                  10000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  1,           /* priority of the task */
                  &Input_Hander,      /* Task handle to keep track of created task */
                  1);          /* pin task to core 1 */
  Serial.println("READY");
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Attempting to reconnect...");
    WiFi.mode(WIFI_STA);
    WiFi.hostname("TENS4VR");
    WiFi.disconnect(true);
    WiFi.begin(ssid, password);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    int tryCount = 0;
    while (WiFi.status() != WL_CONNECTED) {
      if (tryCount > 60) {
        ESP.restart();
      }
      delay(500);
      Serial.print(".");
      tryCount++;
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  checkWiFiConnection();
  OscWiFi.update();
}

void inputLoop( void * pvParameters ) {
  for(;;) {
    if (ActiveCommand != Last_Command) {
      if (ActiveCommand == 55) { // Manual Power Toggle (For ReSync)
        Pause_Commands = true;
        digitalWrite(POWER_BUTTON, HIGH);
        digitalWrite(POWER_BUTTON, LOW);
        delay(3000);
        digitalWrite(POWER_BUTTON, HIGH);
        delay(3000);
        Pause_Commands = false;
      } else if (ActiveCommand == 51) { // Preset Up
        Pause_Commands = true;
        digitalWrite(PUP_BUTTON, HIGH);
        digitalWrite(PUP_BUTTON, LOW);
        delay(BUTTON_DEBOUNCE_DELAY);
        digitalWrite(PUP_BUTTON, HIGH);
        Current_Level = 0;
        delay(BUTTON_DEBOUNCE_DELAY);
        Pause_Commands = false;
      } else if (ActiveCommand == 50) { // Preset Down
        Pause_Commands = true;
        digitalWrite(PDOWN_BUTTON, HIGH);
        digitalWrite(PDOWN_BUTTON, LOW);
        delay(BUTTON_DEBOUNCE_DELAY);
        digitalWrite(PDOWN_BUTTON, HIGH);
        Current_Level = 0;
        delay(BUTTON_DEBOUNCE_DELAY);
        Pause_Commands = false;
      } else if (ActiveCommand == 54) { // Resync Intensity
        Pause_Commands = true;
        digitalWrite(CHANNEL_A_RELAY, RELAY_OFF_STATE);
        digitalWrite(CHANNEL_B_RELAY, RELAY_OFF_STATE);
        Current_Active = false;
        for (int i = 0; i < MAX_INTENSITY; i++) {
          digitalWrite(DOWN_BUTTON, HIGH);
          digitalWrite(DOWN_BUTTON, LOW);
          delay(BUTTON_DEBOUNCE_DELAY);
          digitalWrite(DOWN_BUTTON, HIGH);
          delay(BUTTON_DEBOUNCE_DELAY);
        }
        Current_Level = 0;
        delay(1000);
        Pause_Commands = false;
      }
      Last_Command = ActiveCommand;
    }
    if (Serial.available()) {
      static String receivedMessage = "";
      char c;
      bool messageStarted = false;

      while (Serial.available()) {
        c = Serial.read();
        if (c == '\n') {
          if (!receivedMessage.isEmpty()) {
            int delimiterIndex = receivedMessage.indexOf("::");
            if (delimiterIndex != -1) {
              int headerIndex = receivedMessage.indexOf("::");
              String header = receivedMessage.substring(0, headerIndex);
              if (header == "EN") {
                int valueIndex = receivedMessage.indexOf("::", headerIndex + 2);
                String valueString = receivedMessage.substring(headerIndex + 2, valueIndex);
                int valueInt = valueString.toInt();
                if (valueInt == 1) {
                  TENS_Active = true;
                } else if (valueInt == 0) {
                  TENS_Active = false;
                }
              } else if (header == "ES") {
                int valueIndex = receivedMessage.indexOf("::", headerIndex + 2);
                String valueString = receivedMessage.substring(headerIndex + 2, valueIndex);
                int valueInt = valueString.toInt();
                if (valueInt == 1) {
                  TENS_ESTOP = true;
                } else if (valueInt == 0) {
                  TENS_ESTOP = false;
                }
              } else if (header == "LV") {
                int valueIndex = receivedMessage.indexOf("::", headerIndex + 2);
                String valueString = receivedMessage.substring(headerIndex + 2, valueIndex);
                int valueInt = valueString.toInt();
                if (valueInt >= 0 && valueInt <= MAX_INTENSITY) {
                  TENS_Level = valueInt;
                }
              } else if (header == "PWR") {
                #ifdef POWER_BUTTON
                  digitalWrite(POWER_BUTTON, HIGH);
                  digitalWrite(POWER_BUTTON, LOW);
                  delay(5000);
                  digitalWrite(POWER_BUTTON, HIGH);
                #endif
              } else if (header == "PS") {
                #ifdef PUP_BUTTON
                #ifdef PDOWN_BUTTON
                int valueIndex = receivedMessage.indexOf("::", headerIndex + 2);
                String valueString = receivedMessage.substring(headerIndex + 2, valueIndex);
                int valueInt = valueString.toInt();
                if (valueInt == 1) {
                  Pause_Commands = true;
                  digitalWrite(PUP_BUTTON, HIGH);
                  digitalWrite(PUP_BUTTON, LOW);
                  delay(BUTTON_DEBOUNCE_DELAY);
                  digitalWrite(PUP_BUTTON, HIGH);
                  Current_Level = 0;
                  delay(BUTTON_DEBOUNCE_DELAY);
                  Pause_Commands = false;
                } else if (valueInt == 0) {
                  Pause_Commands = true;
                  digitalWrite(PDOWN_BUTTON, HIGH);
                  digitalWrite(PDOWN_BUTTON, LOW);
                  delay(BUTTON_DEBOUNCE_DELAY);
                  digitalWrite(PDOWN_BUTTON, HIGH);
                  Current_Level = 0;
                  delay(BUTTON_DEBOUNCE_DELAY);
                  Pause_Commands = false;
                }
                #endif
                #endif
              }
            }
          }
          receivedMessage = "";
        } else {
          receivedMessage += c;
        }

      }
    } else {
      delay(1);
    }
  }
}

unsigned long startOp = 0;
unsigned long previousMillis = 0;
void outputLoop( void * pvParameters ) {
  for(;;) {
    if (TENS_ESTOP == true && Current_Active == true) {
      digitalWrite(CHANNEL_A_RELAY, RELAY_OFF_STATE);
      digitalWrite(CHANNEL_B_RELAY, RELAY_OFF_STATE);
      Current_Active = false;
      Serial.println("OUTPUT (OFF) ESTOP");
    }
    if (Current_Active == true && TENS_Active == false && Pause_Commands == false) {
      digitalWrite(CHANNEL_A_RELAY, RELAY_OFF_STATE);
      digitalWrite(CHANNEL_B_RELAY, RELAY_OFF_STATE);
      Current_Active = TENS_Active;
      Serial.println("OUTPUT (OFF)");
    }
    if (TENS_Level <= MAX_INTENSITY && Pause_Commands == false) {
      if (Current_Level < TENS_Level) {
        if (startOp == 0) {
          startOp = millis();
        }
        digitalWrite(UP_BUTTON, HIGH);
        digitalWrite(UP_BUTTON, LOW);
        delay(BUTTON_DEBOUNCE_DELAY);
        digitalWrite(UP_BUTTON, HIGH);
        delay(BUTTON_DEBOUNCE_DELAY);
        Current_Level++;
        Serial.print("LEVEL (+) ");
        Serial.println(Current_Level);
        if (Current_Level == TENS_Level && startOp != 0) {
          Serial.print("LEVEL (=) ");
          long completed = (millis() - startOp);
          Serial.println(completed);
          startOp = 0;
        }
      } else if (Current_Level > TENS_Level) {
        if (startOp == 0) {
          startOp = millis();
        }
        digitalWrite(DOWN_BUTTON, HIGH);
        digitalWrite(DOWN_BUTTON, LOW);
        delay(BUTTON_DEBOUNCE_DELAY);
        digitalWrite(DOWN_BUTTON, HIGH);
        delay(BUTTON_DEBOUNCE_DELAY);
        Current_Level--;
        Serial.print("LEVEL (-) ");
        Serial.println(Current_Level);
        if (Current_Level == TENS_Level && startOp != 0) {
          Serial.print("LEVEL (=) ");
          long completed = (millis() - startOp);
          Serial.println(completed);
          startOp = 0;
        }
      } else {
        if (Current_Active == false && TENS_Active == true && Current_Level == TENS_Level && TENS_ESTOP == false) {
          digitalWrite(CHANNEL_A_RELAY, RELAY_ON_STATE);
          digitalWrite(CHANNEL_B_RELAY, RELAY_ON_STATE);
          Current_Active = TENS_Active;
          Serial.println("OUTPUT (ON)");
        }
        delay(1);
      }
    } else {
      delay(1);
    }
    // Keep Device Alive
    if (millis() - previousMillis >= 5000) {
      previousMillis = millis();
      digitalWrite(POWER_BUTTON, HIGH);
      digitalWrite(POWER_BUTTON, LOW);
      delay(BUTTON_DEBOUNCE_DELAY);
      digitalWrite(POWER_BUTTON, HIGH);
      delay(BUTTON_DEBOUNCE_DELAY);
    }
  }
}