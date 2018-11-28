#include <LowPower.h>
#include <SevenSeg.h>

int digitPins[3] = {A2, A1, A0};
SevenSeg disp(2, 3, 6, 5, 8, 4, 9);
char arr[4] = {'1', '2', '3', '4'};
char d[6] = "0.0.0";
const char *blank = "   ";

unsigned long transitionSeed() {
  unsigned long seed = 0;
  unsigned int i = 0;
  while (i < sizeof(unsigned long) * 8) {
    int r = analogRead(0);
    int x = r ^ analogRead(0);
    while (x) {
      if (x & 1) {
        seed <<= 1;
        seed |= r & 1;
        i++;
      }
      x >>= 1;
      r >>= 1;
    }
  }
  return seed;
}

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, INPUT_PULLUP);
  // Serial.begin(9600);
  Serial.println("Gathering entropy");
  randomSeed(transitionSeed());
  Serial.println("Entropy gathered");
  disp.setCommonAnode();
  disp.setDigitPins(3, digitPins);
  disp.setDPPin(7);
  disp.setTimer(2);
  disp.startTimer();
}

void changeNumber(int changeDelay, int attemptsPerDigit) {
  // First do a full shuffle of array. The array now contains our final number.
  for (int mx = 3; mx >= 1; mx--) {
    int x = random(mx + 1);
    int winner = arr[x];
    arr[x] = arr[mx];
    arr[mx] = winner;
  }
  Serial.println("===");

  // Run through a whole bunch of values on the digits as if brute forcing a
  // code, ala Hollywood.
  for (int scrambleRange = 0; scrambleRange <= 4; scrambleRange += 2) {
    Serial.println("---");
    // One-at-a-time, lock each digit into its final value. Do attemptsPerDigit
    // different "tries" before locking down the digit.
    for (int attempt = 0; attempt < attemptsPerDigit; attempt++) {
      // Make one trip through the digits, assigning each unlocked digit a
      // random value different from its current value
      // by adding [1, 3] and modding.
      for (int digit = scrambleRange; digit <= 4; digit += 2) {
        int value = d[digit] - '1';
        value += random(1, 4);
        value %= 4;
        d[digit] = value + '1';
      }
      // Display it for changeDelay milliseconds.
      disp.write(d);
      Serial.println((char *)d);
      delay(changeDelay);
    }
    // One last thing: write the real value into the front digit.
    d[scrambleRange] = arr[scrambleRange / 2];
  }
}

void wakeUp() {}

void loop() {
  changeNumber(40, 20);

  while (digitalRead(10) == HIGH) {
    delay(5);
  }
  Serial.println("Sleeping zzz");
  disp.stopTimer();
  delay(5);
  digitalWrite(A2, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A0, LOW);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), wakeUp, RISING);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  detachInterrupt(digitalPinToInterrupt(2));
  pinMode(2, OUTPUT);
  disp.startTimer();
  Serial.println("Wake!");
}

ISR(TIMER2_COMPA_vect) { disp.interruptAction(); }
