// AJ's Puzzle Game with Touch Sensor Completion Logic

// Pin Definitions
const int touchSensorPin = 6; // Touch sensor pin
const int speakerPin = 10;    // Speaker pin
const int rgbRedPin = 7;      // RGB LED Red pin
const int rgbGreenPin = 8;    // RGB LED Green pin
const int rgbBluePin = 9;     // RGB LED Blue pin
const int buttonPins[] = {2, 3, 4}; // Pins for buttons
const int numButtons = 3;
const int pulseSensorPin = A0; // Pulse sensor pin

// Time Limits
const unsigned long puzzleTimeLimit = 180000; // 3 minutes
const unsigned long codeTimeLimit = 20000;   // 20 seconds

// Game States
enum GameState { WAIT_FOR_START, PUZZLE_SOLVING, CODE_ENTRY, GAME_OVER, GAME_SUCCESS };
GameState currentState = WAIT_FOR_START;

// Timers
unsigned long stateStartTime = 0;
unsigned long lastButtonPressTime[numButtons] = {0}; // For debounce
const unsigned long debounceDelay = 50;             // 50ms debounce delay

// Code Sequence
const int codeSequence[] = {0, 1, 2}; // Button indices (0-based)
const int pressCounts[] = {2, 1, 3};  // Press counts for each step
const int codeLength = 3;
int buttonPressCounts[numButtons] = {0};
bool buttonStates[numButtons] = {false};
int currentStep = 0;

void setup() {
  Serial.begin(9600);

  // Pin Initialization
  pinMode(touchSensorPin, INPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(rgbRedPin, OUTPUT);
  pinMode(rgbGreenPin, OUTPUT);
  pinMode(rgbBluePin, OUTPUT);
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  Serial.println("AJ's Puzzle Game Initialized. Touch the sensor to start.");
}

void loop() {
  unsigned long currentTime = millis(); // Current time

  switch (currentState) {
    case WAIT_FOR_START:
      handleWaitForStart(currentTime);
      break;

    case PUZZLE_SOLVING:
      handlePuzzleSolving(currentTime);
      break;

    case CODE_ENTRY:
      handleCodeEntry(currentTime);
      break;

    case GAME_OVER:
      handleGameOver();
      break;

    case GAME_SUCCESS:
      handleGameSuccess();
      break;
  }
}

// State Handlers
void handleWaitForStart(unsigned long currentTime) {
  int touchState = digitalRead(touchSensorPin);

  if (touchState == HIGH) {
    currentState = PUZZLE_SOLVING;
    stateStartTime = currentTime;
    playSound(1000, 200); // Short sound
    Serial.println("Game started! Solve the puzzle within 3 minutes.");
  }
}

void handlePuzzleSolving(unsigned long currentTime) {
  unsigned long elapsedTime = currentTime - stateStartTime;
  int touchState = digitalRead(touchSensorPin);

  if (elapsedTime >= puzzleTimeLimit) {
    Serial.println("Time's up! Game over.");
    flashRed();
    playSound(400, 500); // Low failure sound
    currentState = GAME_OVER;
    return;
  }

  if (touchState == HIGH) {
    currentState = CODE_ENTRY;
    stateStartTime = currentTime;
    flashBlue();
    playSound(1200, 300); // Short success sound
    Serial.println("Puzzle complete! Enter the code within 20 seconds.");
  }
}

void handleCodeEntry(unsigned long currentTime) {
  unsigned long elapsedTime = currentTime - stateStartTime;

  // Check if the user runs out of time
  if (elapsedTime >= codeTimeLimit) {
    Serial.println("Code time expired! Game over.");
    flashRed();
    playSound(400, 500); // Low failure sound
    currentState = GAME_OVER;
    return;
  }

  // Process button presses with debounce
  for (int i = 0; i < numButtons; i++) {
    int buttonState = digitalRead(buttonPins[i]);

    // Debounce logic: Ignore rapid toggling
    if (buttonState == LOW && (currentTime - lastButtonPressTime[i] > debounceDelay)) {
      lastButtonPressTime[i] = currentTime;

      if (!buttonStates[i]) {
        buttonStates[i] = true; // Mark button as pressed
        buttonPressCounts[i]++;

        Serial.print("Button ");
        Serial.print(i + 1);
        Serial.print(" pressed! Total presses: ");
        Serial.println(buttonPressCounts[i]);

        // Check if the current button matches the expected sequence
        if (i == codeSequence[currentStep]) {
          if (buttonPressCounts[i] == pressCounts[currentStep]) {
            // Correct number of presses for the current step
            currentStep++; // Move to the next step

            // Check if all steps are complete
            if (currentStep == codeLength) {
              Serial.println("Code correct! Game won!");
              flashGreen();
              playSound(1500, 300);
              currentState = GAME_SUCCESS;
              return;
            }
          } else if (buttonPressCounts[i] > pressCounts[currentStep]) {
            // Too many presses for the current step
            Serial.println("Too many presses! Game over.");
            flashRed();
            playSound(400, 500);
            currentState = GAME_OVER;
            return;
          }
        } else {
          // Button press doesn't match the expected sequence
          Serial.println("Wrong button pressed! Game over.");
          flashRed();
          playSound(400, 500);
          currentState = GAME_OVER;
          return;
        }
      }
    }

    if (buttonState == HIGH && buttonStates[i]) {
      buttonStates[i] = false; // Reset button state when released
    }
  }
}

void handleGameOver() {
  Serial.println("Game Over. Resetting...");
  resetGame();
}

void handleGameSuccess() {
  Serial.println("Congratulations! You won!");
  displayHeartRate();
  resetGame();
}

// Utility Functions
void playSound(int frequency, int duration) {
  tone(speakerPin, frequency, duration);
  delay(duration + 100);
}

void flashRed() {
  setColor(255, 0, 0);
  delay(500);
  setColor(0, 0, 0);
}

void flashBlue() {
  setColor(0, 0, 255);
  delay(500);
  setColor(0, 0, 0);
}

void flashGreen() {
  setColor(0, 255, 0);
  delay(500);
  setColor(0, 0, 0);
}

void setColor(int red, int green, int blue) {
  analogWrite(rgbRedPin, red);
  analogWrite(rgbGreenPin, green);
  analogWrite(rgbBluePin, blue);
}

void displayHeartRate() {
  int pulseValue = analogRead(pulseSensorPin);
  int heartRate = map(pulseValue, 0, 1023, 60, 100); // Example mapping
  Serial.print("Your heart rate: ");
  Serial.print(heartRate);
  Serial.println(" BPM.");
}

void resetGame() {
  currentState = WAIT_FOR_START;
  stateStartTime = 0;
  currentStep = 0;
  for (int i = 0; i < numButtons; i++) {
    buttonPressCounts[i] = 0;
  }
  Serial.println("Game reset. Touch the sensor to restart.");
}
