//www.elegoo.com
//2016.12.9

const int tDelay = 400;
const int latchPin = 10;      // (11) ST_CP [RCK] on 74HC595
const int clockPin = 13;      // (9) SH_CP [SCK] on 74HC595
const int dataPin = 11;     // (12) DS [S1] on 74HC595

const int digitPin0 = 4;
const int digitPin1 = 5;

void setup() {
  Serial.begin(115200);
  while (!Serial); // for Leonardo/Micro

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  pinMode(digitPin0, OUTPUT);
  pinMode(digitPin1, OUTPUT);
  digitalWrite(digitPin0, LOW);
  digitalWrite(digitPin1, HIGH);

  Serial.println("74HC595: ready");
}

void loop() {
  updateShiftRegister(0);
  delay(tDelay);

  //singleUp();
  coverUp();
  delay(5*tDelay);
  openDown();
}

void updateShiftRegister(byte leds) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, leds);
  digitalWrite(latchPin, HIGH);
}

void singleUp() {
  Serial.println("singleUp()");
  int leds = 1;
  for (int i = 0; i < 8; i++) {
    updateShiftRegister(leds);
    delay(tDelay);
    leds <<= 1;
  }
}

void coverUp() {
  Serial.println("coverUp()");
  int leds = 1;
  for (int i = 0; i < 8; i++) {
    updateShiftRegister(leds);
    delay(tDelay);
    leds <<= 1;
    leds |= 1;
  }
}

void coverDown() {
  Serial.println("coverDown()");
  int leds = (1 << 7);
  for (int i = 0; i < 7; i++) {
    updateShiftRegister(leds);
    delay(tDelay);
    leds >>= 1;
    leds |= 0x80;
  }
}

void openDown() {
  Serial.println("openDown()");
  int leds = 0xff;
  for (int i = 0; i < 8; i++) {
    updateShiftRegister(leds);
    delay(tDelay);
    leds >>= 1;
  }
}

