#include <WiFi.h>

const char *ssid = "yale wireless";
const char *serverIP = "193.122.147.106";
const uint16_t serverPort = 7531;

const int relayPin = 23;

WiFiClient client;
bool isRed = false;
bool lastState = false;

void connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

bool connectServer()
{
  if (client.connected())
    return true;

  Serial.println("Connecting to server...");
  if (client.connect(serverIP, serverPort))
  {
    client.print("lights");
    Serial.println("Connected to server!");
    return true;
  }
  else
  {
    Serial.println("Server connection failed.");
    return false;
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  connectWiFi();
  connectServer();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  if (!connectServer())
  {
    delay(5000);
    return;
  }

  while (client.available())
  {
    String command = client.readStringUntil('\n');
    command.trim();

    Serial.println("Received: " + command);

    if (command == "white")
    {
      isRed = false;
    }
    else if (command == "red")
    {
      isRed = true;
    }
  }

  digitalWrite(relayPin, isRed ? LOW : HIGH);
}