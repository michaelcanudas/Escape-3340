#include <WiFi.h>

const char *ssid = "yale wireless";

const char *serverIP = "10.66.236.77";
const uint16_t serverPort = 7531;

const int relayPin = 23;
bool on = false;

WiFiClient client;

void setup()
{
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

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
  }
  else
  {
    Serial.println("Connection failed.");
  }
}

void loop()
{
  if (!client.connected())
  {
    Serial.println("Disconnected. Reconnecting...");
    if (client.connect(serverIP, serverPort))
    {
      Serial.println("Reconnected!");
    }
    else
    {
      delay(5000); // Wait 5 seconds before retry
      return;
    }
  }

  if (client.available())
  {
    String command = client.readStringUntil('\n');
    command.trim();

    Serial.println("Received: " + command);

    if (command == "toggle")
    {
      on = !on;
    }

    if (on)
    {
      digitalWrite(relayPin, HIGH);
    }
    else
    {
      digitalWrite(relayPin, LOW);
    }
  }
}
