#include <WiFi.h>

const char *ssid = "yale wireless";

const char *serverIP = "10.66.236.77"; // Pi IP address
const uint16_t serverPort = 7531;

const int relayPin = 23;

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

  // Try to connect to the server
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
    String cmd = client.readStringUntil('\n');
    cmd.trim();
    Serial.println("Received: " + cmd);

    if (cmd == "on:on")
    {
      digitalWrite(relayPin, HIGH);
      Serial.println("Lights ON");
    }
    else if (cmd == "off:off")
    {
      digitalWrite(relayPin, LOW);
      Serial.println("Lights OFF");
    }
  }
}
