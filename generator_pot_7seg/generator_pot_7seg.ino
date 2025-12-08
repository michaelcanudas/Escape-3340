#include <WiFi.h>

// ================= NETWORK CONFIGURATION =================
const char *ssid = "yale wireless";
const char *serverIP = "193.122.147.106";
const uint16_t serverPort = 7531;

WiFiClient client;
const int relayPin = 23; // Kept from netcode
bool isRed = false;      // Kept from netcode logic

// ================= DISPLAY CONFIGURATION =================
// Pins from your EXISTING CODE
const int segA = 12;
const int segB = 14;
const int segC = 25;
const int segD = 26;
const int segE = 27;
const int segF = 33;
const int segG = 32;

const int pot = 34;

const int segmentPins[] = {segA, segB, segC, segD, segE, segF, segG};

// Potentiometer thresholds
const int detents[] = {
  3200, // Star
  1650, // Bow
  300, // Mistletoe
  0     // Snowflake
};

// Digit patterns (0-9)
const byte digitPatterns[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

// ================= STATE MANAGEMENT =================
bool isSpinnerMode = false; // False = Normal Display, True = Spinner

// Variables for non-blocking spinner animation
unsigned long lastSpinTime = 0;
int spinIndex = 0;
const int spinDelay = 100; // Speed of the spinner in ms

// ================= SETUP FUNCTIONS =================

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

bool connectServer() {
  if (client.connected()) return true;

  Serial.println("Connecting to server...");
  if (client.connect(serverIP, serverPort)) {
    client.print("generator_pot_7seg"); // Handshake expected by server
    Serial.println("Connected to server!");
    return true;
  } else {
    Serial.println("Server connection failed.");
    return false;
  }
}

void setup() {
  Serial.begin(115200);

  // Pin Setup
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  
  pinMode(pot, INPUT);
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW);
  }

  // Network Setup
  connectWiFi();
  connectServer();
}

// ================= HELPER FUNCTIONS =================

// Standard helper to write a digit
void displayDigit(int digit) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], digitPatterns[digit][i]);
  }
}

// Helper to handle the "Normal" logic (Potentiometer reading)
void runNormalDisplay() {
  int potRead = analogRead(pot);
  // Serial.println(potRead);

  // Logic from existing code
  if(potRead > detents[0]) {
    displayDigit(4);
  } else if(potRead > detents[1]) {
    displayDigit(9);
  } else if(potRead > detents[2]) {
    displayDigit(0);
  } else {
    displayDigit(3);
  }
  // No delay here to keep loop fast for network reading
}

// Helper to handle the "Spinner" animation
void runSpinner() {
  if (millis() - lastSpinTime > spinDelay) {
    lastSpinTime = millis();
    
    // Clear all segments first
    for(int i=0; i<7; i++) digitalWrite(segmentPins[i], LOW);

    // Turn on the current segment in the sequence
    // Order: A(0) -> B(1) -> C(2) -> D(3) -> E(4) -> F(5) -> Loop
    digitalWrite(segmentPins[spinIndex], HIGH);

    spinIndex++;
    if (spinIndex > 5) spinIndex = 0; // Skip index 6 (G/Middle)
  }
}

// ================= MAIN LOOP =================

void loop() {
  // 1. Maintain Network Connection
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!connectServer()) {
    delay(5000); // Wait before retrying server
    return;
  }

  isSpinnerMode = false; // Reset mode if wifi drops

  // 2. Process Incoming Commands
  while (client.available()) {
    String command = client.readStringUntil('\n');
    command.trim();

    Serial.println("Received: " + command);

    // Mode Switching Commands
    if (command == "spinner") {
      isSpinnerMode = true;
      // Reset spin state so it starts fresh
      spinIndex = 0;
      for(int i=0; i<7; i++) digitalWrite(segmentPins[i], LOW);
    } 
    else if (command == "display") {
      isSpinnerMode = false;
    }
    
    // Original Relay Commands (Folded in)
    else if (command == "white") {
      isRed = false;
      digitalWrite(relayPin, HIGH);
    } 
    else if (command == "red") {
      isRed = true;
      digitalWrite(relayPin, LOW);
    }
  }

  // 3. Execute Display Logic based on Mode
  if (isSpinnerMode) {
    runSpinner();
  } else {
    runNormalDisplay();
  }
  
  // Small delay to prevent CPU hogging, but fast enough for smooth animation
  delay(10); 
}