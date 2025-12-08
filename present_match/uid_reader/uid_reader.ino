#include <Arduino.h>
#include <SPI.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

const uint8_t READERS_COUNT = 5;
const uint8_t sdaPins[READERS_COUNT] = {4, 5, 17, 21};
const uint8_t RST_PIN = 22;

MFRC522DriverPinSimple* readerPins[READERS_COUNT];
MFRC522DriverSPI* readerDrivers[READERS_COUNT];
MFRC522* readers[READERS_COUNT];

void setup() {
	Serial.begin(115200);
	SPI.begin();

	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		readerPins[i] = new MFRC522DriverPinSimple(sdaPins[i]);
		readerDrivers[i] = new MFRC522DriverSPI(*readerPins[i]);
		readers[i] = new MFRC522(*readerDrivers[i]);
		readers[i]->PCD_Init();
	}
}

void loop() {
	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		MFRC522* reader = readers[i];
		
		if (reader->PICC_IsNewCardPresent() && reader->PICC_ReadCardSerial()) {
			Serial.print(F("Reader "));
			Serial.print(i);
			Serial.print(F(" UID:"));
			
			for (byte b = 0; b < reader->uid.size; b++) {
				if (reader->uid.uidByte[b] < 0x10) Serial.print(F(" 0"));
				else Serial.print(F(" "));
				Serial.print(reader->uid.uidByte[b], HEX);
			}
			Serial.println();
			
			reader->PICC_HaltA();
			reader->PCD_StopCrypto1();
		}
	}
}