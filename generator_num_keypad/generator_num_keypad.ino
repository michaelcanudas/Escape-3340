#include <WiFi.h>

const char *ssid = "yale wireless";
const char *serverIP = "193.122.147.106";
const uint16_t serverPort = 7531;

WiFiClient client;

const int keyPins[] = {16, 32, 33, 25, 26, 27, 14, 12, 13, 15, 2, 17};
const char keyLabels[] = {'*', '7', '4', '1', '0', '8', '5', '2', '#', '9', '6', '3'};
const int keyCount = sizeof(keyPins) / sizeof(keyPins[0]);
const int hallPin = 23;

int lastKeyState[12];
int lastMagnetState = -1; 
char sequence[4] = {'-', '-', '-', '-'};

void setup() {
  Serial.begin(115200);
  pinMode(hallPin, INPUT);

  for (int i = 0; i < keyCount; i++) {
    pinMode(keyPins[i], INPUT_PULLUP);
    lastKeyState[i] = HIGH;
  }

  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  if (client.connect(serverIP, serverPort)) {
    client.print("generator_connected");
  }
}

void loop() {
  if (!client.connected()) {
    if (client.connect(serverIP, serverPort)) {
      client.print("generator_reconnected");
    } else {
      delay(500); 
      return;
    }
  }

  // KEYPAD LOGIC
  for (int i = 0; i < keyCount; i++) {
    int currentState = digitalRead(keyPins[i]);

    if (currentState == LOW && lastKeyState[i] == HIGH) {
      char pressed = keyLabels[i];
      
      if (sequence[3] != pressed) {
        sequence[0] = sequence[1];
        sequence[1] = sequence[2];
        sequence[2] = sequence[3];
        sequence[3] = pressed;

        String msg = "event:generator_code:";
        msg += sequence[0];
        msg += sequence[1];
        msg += sequence[2];
        msg += sequence[3];
        
        client.print(msg);
        Serial.println(msg);
      }
      delay(50);
    }
    lastKeyState[i] = currentState;
  }

  // MAGNET LOGIC
  int currentMagnetState = digitalRead(hallPin);
  
  if (currentMagnetState != lastMagnetState) {
    String magnetMsg = "event:generator_magnet:";
    // If LOW, magnet is detected (true). If HIGH, not detected (false).
    if (currentMagnetState == LOW) {
      magnetMsg += "true";
    } else {
      magnetMsg += "false";
    }
    
    client.print(magnetMsg);
    Serial.println(magnetMsg);
    
    lastMagnetState = currentMagnetState;
    delay(50); 
  }
}