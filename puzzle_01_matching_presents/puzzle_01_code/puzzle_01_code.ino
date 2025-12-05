#include <Arduino.h>
#include <SPI.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

const uint8_t READERS_COUNT = 5;
const uint8_t sdaPins[READERS_COUNT] = {4, 5, 17, 21, 2};
const uint8_t RST_PIN = 22;

const byte TARGET_UIDS[READERS_COUNT][7] = {
	{0x04, 0x33, 0x62, 0xCC, 0x2E, 0x61, 0x80},
	{0x04, 0xC2, 0x0D, 0x72, 0x3E, 0x61, 0x80},
	{0x04, 0x0C, 0x41, 0xCE, 0x2E, 0x61, 0x80},
	{0x04, 0x2C, 0x56, 0x6F, 0x3E, 0x61, 0x80},
	{0x04, 0x56, 0x16, 0x66, 0x4E, 0x61, 0x80}
};

byte lastReadUIDs[READERS_COUNT][7]; // Stores the last UID seen by each reader
bool puzzleActive[READERS_COUNT];
bool puzzleSolved = false;

MFRC522DriverPinSimple* readerPins[READERS_COUNT];
MFRC522DriverSPI* readerDrivers[READERS_COUNT];
MFRC522* readers[READERS_COUNT];

void onPuzzleSolve() {
	if (!puzzleSolved) {
		Serial.println(F("PUZZLE SOLVED!"));
		puzzleSolved = true;
	}
}

bool checkUID(byte* readUID, const byte* targetUID) {
	for (byte i = 0; i < 7; i++) {
		if (readUID[i] != targetUID[i]) return false;
	}
	return true;
}

void checkPuzzle() {
	bool allCorrect = true;
	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		// Compare the last known UID on this reader to the target
		if (checkUID(lastReadUIDs[i], TARGET_UIDS[i])) {
			puzzleActive[i] = true;
		} else {
			puzzleActive[i] = false;
			allCorrect = false;
		}
	}

	if (allCorrect) {
		onPuzzleSolve();
	} else {
		puzzleSolved = false;
	}
}

void setup() {
	Serial.begin(115200);
	SPI.begin();

	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		readerPins[i] = new MFRC522DriverPinSimple(sdaPins[i]);
		
		// Initialize with a slower SPI speed (500kHz) to fix stability issues
		// MFRC522DriverSPI(pin, spi_interface, spi_settings)
		readerDrivers[i] = new MFRC522DriverSPI(
			*readerPins[i], 
			SPI, 
			SPISettings(500000, MSBFIRST, SPI_MODE0)
		);
		
		readers[i] = new MFRC522(*readerDrivers[i]);
		readers[i]->PCD_Init();
		
		puzzleActive[i] = false;
		memset(lastReadUIDs[i], 0, 7); // Clear history on boot
	}
}

void loop() {
	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		MFRC522* reader = readers[i];
		
		if (reader->PICC_IsNewCardPresent() && reader->PICC_ReadCardSerial()) {
			// Update the persistent memory for this reader
			memcpy(lastReadUIDs[i], reader->uid.uidByte, 7);
			
			Serial.print(F("Reader "));
			Serial.print(i);
			Serial.println(F(" Updated"));
			
			reader->PICC_HaltA();
			reader->PCD_StopCrypto1();
		}
	}
	checkPuzzle();

	static unsigned long lastPrintTime = 0;
	if (millis() - lastPrintTime > 100) {
		lastPrintTime = millis();
		Serial.print(F("Status: "));
		for (uint8_t i = 0; i < READERS_COUNT; i++) {
			Serial.print(F("R"));
			Serial.print(i);
			Serial.print(F(":"));
			Serial.print(puzzleActive[i] ? "OK " : "-- ");
		}
		Serial.println(puzzleSolved ? F("SOLVED") : F("WAITING"));
	}
}