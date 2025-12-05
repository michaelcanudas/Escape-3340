#include <Arduino.h>
#include <SPI.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <WiFi.h>

// --- WiFi & Network Configuration ---
const char *ssid = "yale wireless";
const char *serverIP = "10.66.236.77"; // Pi IP address
const uint16_t serverPort = 7531;

WiFiClient client;

// --- RFID Configuration ---
const uint8_t READERS_COUNT = 4;
const uint8_t sdaPins[READERS_COUNT] = {4, 5, 17, 21};
const uint8_t RST_PIN = 22;

byte lastReadUIDs[READERS_COUNT][7]; // Stores the last UID seen by each reader

MFRC522DriverPinSimple* readerPins[READERS_COUNT];
MFRC522DriverSPI* readerDrivers[READERS_COUNT];
MFRC522* readers[READERS_COUNT];

// Construct the status string and send to server
void onGameStatusChange() {
	if (client.connected()) {
		String msg = "status:presents";
		
		for (uint8_t i = 0; i < READERS_COUNT; i++) {
			msg += ":";
			for (int b = 0; b < 7; b++) {
				if (lastReadUIDs[i][b] < 0x10) msg += "0";
				msg += String(lastReadUIDs[i][b], HEX);
			}
		}
		
    client.println("relay:toggle");
		client.println(msg);
		Serial.println("Sent: " + msg);
	}
}

void setup() {
	Serial.begin(115200);
	SPI.begin();

	// --- WiFi Setup ---
	WiFi.begin(ssid);
	Serial.print("Connecting to WiFi");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

	// --- Server Connection ---
	if (client.connect(serverIP, serverPort)) {
		Serial.println("Connected to server!");
		client.println("presents"); // Send handshake on initial connection
	} else {
		Serial.println("Connection failed.");
	}

	// --- RFID Initialization ---
	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		readerPins[i] = new MFRC522DriverPinSimple(sdaPins[i]);
		
		// Initialize with a slower SPI speed (500kHz) to fix stability issues
		readerDrivers[i] = new MFRC522DriverSPI(
			*readerPins[i], 
			SPI, 
			SPISettings(500000, MSBFIRST, SPI_MODE0)
		);
		
		readers[i] = new MFRC522(*readerDrivers[i]);
		readers[i]->PCD_Init();
		
		memset(lastReadUIDs[i], 0, 7); // Clear history on boot
	}
}

void loop() {
	// --- Network Reconnection Logic ---
	if (!client.connected()) {
		Serial.println("Disconnected. Reconnecting...");
		if (client.connect(serverIP, serverPort)) {
			Serial.println("Reconnected!");
			client.println("presents"); // Send handshake on reconnection
		} else {
			delay(5000); // Wait 5 seconds before retry
			return;
		}
	}

	// --- RFID Loop ---
	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		MFRC522* reader = readers[i];
		
		if (reader->PICC_IsNewCardPresent() && reader->PICC_ReadCardSerial()) {
			// Update the persistent memory for this reader
			memcpy(lastReadUIDs[i], reader->uid.uidByte, 7);
			
			Serial.print(F("Reader "));
			Serial.print(i);
			Serial.println(F(" Updated"));
			
			// Send update to server
			onGameStatusChange();

			reader->PICC_HaltA();
			reader->PCD_StopCrypto1();
		}
	}
}