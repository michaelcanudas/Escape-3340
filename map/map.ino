#include <WiFi.h>

const char *ssid = "yale wireless";
const char *serverIP = "193.122.147.106";
const uint16_t serverPort = 7531;

WiFiClient client;

const int NUM_BUTTONS = 5;
const int BUTTON_PINS[NUM_BUTTONS] = {34, 35, 32, 33, 25};
const int SUBMIT_PIN = 26;

int lastSubmitRaw = HIGH;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  if (client.connect(serverIP, serverPort))
  {
    Serial.println("Connected to server!");
    client.print("map");
  }
  else
  {
    Serial.println("Connection failed.");
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT);
  }
  pinMode(SUBMIT_PIN, INPUT_PULLUP);
}

void loop() {
  if (!client.connected())
  {
    Serial.println("Disconnected. Reconnecting...");
    if (client.connect(serverIP, serverPort))
    {
      Serial.println("Reconnected!");
      client.print("map");
    }
    else
    {
      delay(5000);
      return;
    }
  }

  String buttonStates = "";
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int raw = digitalRead(BUTTON_PINS[i]);
    int logicalState = (raw == LOW) ? 1 : 0;
    buttonStates += String(logicalState);
    Serial.print(logicalState);
    Serial.print(" ");
  }

  writeln(buttonStates);

  int submitRaw = digitalRead(SUBMIT_PIN);
  int submitState = (submitRaw == LOW) ? 1 : 0;
  Serial.println(submitState);

  if (submitRaw == LOW && lastSubmitRaw == HIGH) {
    String msg = "event:map_submit:" + buttonStates;
    client.print(msg);
    Serial.println("Sent: " + msg);
  }
  lastSubmitRaw = submitRaw;

  delay(200);
}
