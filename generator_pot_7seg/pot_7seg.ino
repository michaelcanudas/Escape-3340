const int segA = 12;
const int segB = 14;
const int segC = 25;
const int segD = 26;
const int segE = 27;
const int segF = 33;
const int segG = 32;

const int pot = 34;

const int segmentPins[] = {segA, segB, segC, segD, segE, segF, segG};

const int detents[] = {
  3820, // Star
  2550, // Bow
  1050, // Mistletoe
  0 // Snowflake
};

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

void setup() {
  pinMode(pot, INPUT);
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW);
  }
  Serial.begin(115200);
}

void loop() {
  int potRead = analogRead(pot);
  if(potRead > detents[0]) {
    displayDigit(4);
  } else if(potRead > detents[1]) {
    displayDigit(9);
  } else if(potRead > detents[2]) {
    displayDigit(0);
  } else {
    displayDigit(3);
  }
  delay(100);
}

void displayDigit(int digit) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], digitPatterns[digit][i]);
  }
}