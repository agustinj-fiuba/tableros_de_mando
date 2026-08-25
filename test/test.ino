// Pin definitions for ESP32
const int ledPin = 21;      // GPIO 16 (Salida a la LED)
const int buttonPin = 4;   // GPIO 4 (Connect push button to GND)
const int potePin = 34; // GPIO 34 (Potentiometer for interval)

// Time variables
int runTime = 5000;      // Time the LED stays ON (5 seconds)
unsigned long startTime = 0;       // Stores the exact moment the LED turned ON

// State variables
bool isLedOn = false; 
bool lastButtonState = HIGH;       // Stores the previous state of the button
bool waitingForRelease = false;    // Safety lock variable

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT); 
  pinMode(potePin, INPUT);
  Serial.begin(115200);
}

void loop() {
  unsigned long currentTime = millis();
  bool currentButtonState = digitalRead(buttonPin);

  // 1. Release Safety Lock: If the button is released (HIGH), clear the lock
  if (currentButtonState == HIGH) {
    waitingForRelease = false;
  }

  // 2. Edge Detection: Trigger ONLY when pressed, LED is off, AND the lock is clear
  if (currentButtonState == LOW && lastButtonState == HIGH && !isLedOn && !waitingForRelease) {
    digitalWrite(ledPin, HIGH);    // Turn the LED ON
    startTime = currentTime;       // Save the start time
    isLedOn = true;                // Update the LED state
    waitingForRelease = true;      // Activate safety lock immediately

    // Adjust time of interval
    int rawValue = analogRead(potePin);
    int index = map(rawValue, 0, 4095, 1, 10);
    runTime =  index*1000;

    delay(50);                     // Software debounce
  }

  // Save the current state for the next loop iteration
  lastButtonState = currentButtonState;

  // 3. Turn the LED OFF automatically once the timer expires
  if (isLedOn && (currentTime - startTime >= runTime)) {
    digitalWrite(ledPin, LOW);     // Turn the LED OFF
    isLedOn = false;               // Update the LED state
    
    // If you are STILL holding the button when the LED turns off, lock it!
    if (digitalRead(buttonPin) == LOW) {
      waitingForRelease = true;
    }
  }
}

