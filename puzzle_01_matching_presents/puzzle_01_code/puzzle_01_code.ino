#include <Arduino.h>
#include <SPI.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

const uint8_t READERS_COUNT = 3;
const uint8_t sdaPins[READERS_COUNT] = {4, 5, 17};
const uint8_t RST_PIN = 27;

const byte TARGET_UIDS[READERS_COUNT][7] = {
	{0x04, 0x33, 0x62, 0xCC, 0x2E, 0x61, 0x80},
	{0x04, 0xC2, 0x0D, 0x72, 0x3E, 0x61, 0x80},
	{0x04, 0x0C, 0x41, 0xCE, 0x2E, 0x61, 0x80}
	// {0x04, 0x2C, 0x56, 0x6F, 0x3E, 0x61, 0x80},
	// {0x04, 0x56, 0x16, 0x66, 0x4E, 0x61, 0x80}
};

unsigned long lastReadTime[READERS_COUNT];
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
	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		if (millis() - lastReadTime[i] > 500) {
			puzzleActive[i] = false;
		}
	}

	bool allCorrect = true;
	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		if (!puzzleActive[i]) {
			allCorrect = false;
			break;
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
		readerDrivers[i] = new MFRC522DriverSPI(*readerPins[i]);
		readers[i] = new MFRC522(*readerDrivers[i]);
		readers[i]->PCD_Init();
		lastReadTime[i] = 0;
		puzzleActive[i] = false;
	}
}

void loop() {
	for (uint8_t i = 0; i < READERS_COUNT; i++) {
		MFRC522* reader = readers[i];
		
		// Buffer to store the Answer To Request (ATQA)
		byte bufferATQA[2];
		byte bufferSize = sizeof(bufferATQA);

		// Use PICC_WakeupA instead of PICC_IsNewCardPresent.
		// WakeupA sends WUPA (0x52) which forces Halted cards to respond.
		if (reader->PICC_WakeupA(bufferATQA, &bufferSize) == MFRC522::StatusCode::STATUS_OK && reader->PICC_ReadCardSerial()) {
			if (checkUID(reader->uid.uidByte, TARGET_UIDS[i])) {
				lastReadTime[i] = millis();
				puzzleActive[i] = true;
			}
			
			// Halt the card so it must be woken up again next loop
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