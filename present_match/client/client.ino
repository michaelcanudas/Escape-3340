#include <Arduino.h>
#include <SPI.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <WiFi.h>

const char *ssid = "yale wireless";
const char *serverIP = "193.122.147.106";
const uint16_t serverPort = 7531;

WiFiClient client;

const uint8_t READERS_COUNT = 4;
const uint8_t sdaPins[READERS_COUNT] = {4, 5, 17, 21};
const uint8_t RST_PIN = 22;

byte lastReadUIDs[READERS_COUNT][7];

MFRC522DriverPinSimple *readerPins[READERS_COUNT];
MFRC522DriverSPI *readerDrivers[READERS_COUNT];
MFRC522 *readers[READERS_COUNT];

void sendEvent(uint8_t readerId, byte *uid, uint8_t uidSize)
{
	if (!client.connected())
		return;

	String msg = "event:present_tapped:";
	msg += String(readerId);

	msg += ":";
	for (uint8_t i = 0; i < uidSize; i++)
	{
		if (uid[i] < 0x10)
			msg += "0";
		msg += String(uid[i], HEX);
	}

	client.print(msg);
	Serial.println("Sent: " + msg);
}

void setup()
{
	Serial.begin(115200);
	SPI.begin();

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
		client.print("presents");
	}
	else
	{
		Serial.println("Connection failed.");
	}

	for (uint8_t i = 0; i < READERS_COUNT; i++)
	{
		readerPins[i] = new MFRC522DriverPinSimple(sdaPins[i]);
		readerDrivers[i] = new MFRC522DriverSPI(
			*readerPins[i],
			SPI,
			SPISettings(500000, MSBFIRST, SPI_MODE0));
		readers[i] = new MFRC522(*readerDrivers[i]);
		readers[i]->PCD_Init();

		memset(lastReadUIDs[i], 0, 7);
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
			client.print("presents");
		}
		else
		{
			delay(5000);
			return;
		}
	}

	for (uint8_t i = 0; i < READERS_COUNT; i++)
	{
		MFRC522 *reader = readers[i];

		if (reader->PICC_IsNewCardPresent() && reader->PICC_ReadCardSerial())
		{
			memset(lastReadUIDs[i], 0, 7);
			memcpy(lastReadUIDs[i], reader->uid.uidByte, reader->uid.size);

			Serial.print("Reader ");
			Serial.print(i);
			Serial.println(" Updated");

			sendEvent(i, reader->uid.uidByte, reader->uid.size);

			reader->PICC_HaltA();
			reader->PCD_StopCrypto1();
		}
	}
}
