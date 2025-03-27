/*
  Random Name Generator with Tilt Sensor for Arduino Uno R4
  - Displays a random name when button is pressed
  - Shows "Slurker: X" with random number 1-5
  - Adds < > around name if tilted
  
  LCD Connections based on Sparkfun schema:
  https://cdn.sparkfun.com/assets/learn_tutorials/6/3/6/SIK_Circuit_4A.png
  
  Connections:
  - Tilt sensor connected to A0
  - Button connected to digital pin 7 (with 10K pull-up resistor to 5V)
  - Button other terminal to GND
*/

// Husk å installere LiquidCrystal package i Arduino IDE
#include <LiquidCrystal.h>

// Initialize the LCD library with the pins from the Sparkfun schema
// Parameters: (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Pin definitions
const int tiltSensorPin = A0;
const int buttonPin = 3;  // Changed from 7 to 3 as per requirements
const int speakerPin = 7; // Speaker connected to pin 7

// Debug mode
const bool DEBUG_MODE = true;  // Set to false to disable debug output

// Countdown variables
bool countdownActive = false;
unsigned long lastCountdownTime = 0;
const unsigned long countdownInterval = 1000; // 1 second interval

// Tilt threshold
const int TILT_THRESHOLD = 500;  // Values above this are considered "tilted"

// Array of names to choose from
const char* names[] = {
  "Alice", "Bob", "Charlie", "David", "Emma",
  "Frank", "Grace", "Henry", "Isabel", "Jack"
};
const int numNames = 10;  // Number of names in the array

// Variables for tilt detection
int tiltValue = 0;
bool isTilted = false;
bool lastTiltState = false;
unsigned long lastTiltDebounceTime = 0;

// Variables for button handling
int buttonState = HIGH;         // Current state of the button (HIGH = not pressed)
int lastButtonState = HIGH;     // Previous state of the button
unsigned long lastButtonDebounceTime = 0;  // Last time the button state changed

// Debounce time in milliseconds
const unsigned long debounceDelay = 50;

// Current display values
int currentNameIndex = 0;
int currentSlurkerNumber = 1;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  
  // Initialize the random number generator with a random seed
  // Using an unconnected analog pin for randomness
  randomSeed(analogRead(A5));
  
  // Set up button pin as input with internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);
  
  // Set up speaker pin as output
  pinMode(speakerPin, OUTPUT);
  
  // Set up the LCD's number of columns and rows
  lcd.begin(16, 2);
  
  // Display initial message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button to");
  lcd.setCursor(0, 1);
  lcd.print("start...");
  
  Serial.println("Random Name Generator Ready");
  Serial.println("Press the button to generate a random name");
  
  // Select initial random values
  currentNameIndex = random(numNames);
  currentSlurkerNumber = random(1, 6);  // Random number between 1 and 5
  
  delay(1000);
}

void loop() {
  // Read the tilt sensor value
  tiltValue = analogRead(tiltSensorPin);
  
  // Determine if tilted based on threshold
  bool currentTiltState = (tiltValue > TILT_THRESHOLD);
  
  // Check if the tilt state has changed
  if (currentTiltState != lastTiltState) {
    // Reset the debounce timer
    lastTiltDebounceTime = millis();
  }
  
  // If the tilt state has been stable for the debounce period, update the actual tilt state
  if ((millis() - lastTiltDebounceTime) > debounceDelay) {
    // Only update if the state has actually changed
    if (currentTiltState != isTilted) {
      isTilted = currentTiltState;
      
      // If newly tilted, start the countdown
      if (isTilted) {
        countdownActive = true;
        lastCountdownTime = millis();
        if (DEBUG_MODE) {
          Serial.println("Tilt detected! Starting countdown...");
        }
      } else {
        // If no longer tilted, stop the countdown
        countdownActive = false;
        if (DEBUG_MODE) {
          Serial.println("Tilt removed! Stopping countdown.");
        }
      }
      
      // Update the display with the new tilt state
      updateDisplay();
    }
  }
  
  // Save the current tilt state for next comparison
  lastTiltState = currentTiltState;
  
  // Improved button handling
  int reading = digitalRead(buttonPin);
  
  // If the button reading has changed
  if (reading != lastButtonState) {
    // Reset the debounce timer
    lastButtonDebounceTime = millis();
  }
  
  // If the button state has been stable for the debounce period
  if ((millis() - lastButtonDebounceTime) > debounceDelay) {
    // If the button state has changed
    if (reading != buttonState) {
      // Log state transition if debug mode is enabled
      if (DEBUG_MODE) {
        Serial.print("BUTTON STATE CHANGED: ");
        Serial.print(buttonState == LOW ? "PRESSED" : "RELEASED");
        Serial.print(" -> ");
        Serial.println(reading == LOW ? "PRESSED" : "RELEASED");
      }
      
      buttonState = reading;
      
      // If the button is pressed (LOW because of pull-up resistor)
      if (buttonState == LOW) {
        // Play a beep sound when button is pressed
        playButtonSound();
        
        // Visual feedback on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selecting...");
        
        // Short delay for visual feedback
        delay(200);
        
        // Generate new random values
        currentNameIndex = random(numNames);
        currentSlurkerNumber = random(1, 6);  // Random number between 1 and 5
        
        // Update the display with new values
        updateDisplay();
        
        // Debug output if debug mode is enabled
        if (DEBUG_MODE) {
          Serial.print("Button pressed! New name: ");
          Serial.print(names[currentNameIndex]);
          Serial.print(", Slurker number: ");
          Serial.println(currentSlurkerNumber);
          
          // Log button press event with timestamp
          Serial.print("Button press detected at millis(): ");
          Serial.println(millis());
        }
      } else if (DEBUG_MODE) {
        // Log button release event if debug mode is enabled
        Serial.print("Button released at millis(): ");
        Serial.println(millis());
      }
    }
  }
  
  // Save the button reading for next comparison
  lastButtonState = reading;
  
  // Print debug information only when DEBUG_MODE is enabled
  if (DEBUG_MODE) {
    // Print tilt status to serial for debugging
    Serial.print("Tilt value: ");
    Serial.print(tiltValue);
    Serial.print(" | Tilted: ");
    Serial.println(isTilted ? "YES" : "NO");
    
    // Print button status for debugging
    Serial.print("Button raw reading: ");
    Serial.print(reading);
    Serial.print(" | Debounced state: ");
    Serial.print(buttonState == LOW ? "PRESSED" : "RELEASED");
    Serial.print(" | Debounce timer: ");
    Serial.print(millis() - lastButtonDebounceTime);
    Serial.println("ms");
  }
  
  // Handle countdown if active and tilted
  if (countdownActive && isTilted) {
    // Check if it's time to decrement the counter (every second)
    if (millis() - lastCountdownTime >= countdownInterval) {
      // Update the last countdown time
      lastCountdownTime = millis();
      
      // Decrement the counter
      currentSlurkerNumber--;
      
      // Update just the countdown number on the display
      lcd.setCursor(9, 1);  // Position after "Slurker: "
      lcd.print(currentSlurkerNumber);
      lcd.print("   ");  // Clear any extra digits
      
      if (DEBUG_MODE) {
        Serial.print("Countdown: ");
        Serial.println(currentSlurkerNumber);
      }
      
      // If countdown reaches zero, play sound and reset
      if (currentSlurkerNumber <= 0) {
        playSound();
        
        // Reset the counter to a random value between 1 and 5
        currentSlurkerNumber = random(1, 6);
        
        if (DEBUG_MODE) {
          Serial.println("Countdown reached zero! Playing sound.");
        }
      }
    }
  }
  
  // Short delay before next reading
  delay(50);
}

// Function to update the LCD display
void updateDisplay() {
  // Clear the display
  lcd.clear();
  
  // First line: Display the name with or without < > based on tilt state
  lcd.setCursor(0, 0);
  
  // Center the name on the display (16 characters wide)
  String nameDisplay = names[currentNameIndex];
  
  // Add < > if tilted
  if (isTilted) {
    nameDisplay = "<" + nameDisplay + ">";
  }
  
  // Calculate position to center the text
  int startPos = (16 - nameDisplay.length()) / 2;
  if (startPos < 0) startPos = 0;
  
  // Set cursor to centered position
  lcd.setCursor(startPos, 0);
  lcd.print(nameDisplay);
  
  // Second line: Display "Slurker: X"
  lcd.setCursor(0, 1);
  lcd.print("Slurker: ");
  lcd.print(currentSlurkerNumber);
}

// Function to play a sound when countdown reaches zero
void playSound() {
  // Play a simple beep tone
  tone(speakerPin, 1000);  // 1kHz tone
  delay(500);              // Duration of 500ms
  noTone(speakerPin);      // Stop the tone
  
  // Play a second beep (optional)
  delay(100);
  tone(speakerPin, 1200);  // Slightly higher tone
  delay(300);
  noTone(speakerPin);
  
  if (DEBUG_MODE) {
    Serial.println("Sound played!");
  }
}

// Function to play a button click sound
void playButtonSound() {
  // Play a short, higher-pitched beep for button press
  tone(speakerPin, 2000);  // 2kHz tone (higher pitch)
  delay(50);               // Very short duration
  noTone(speakerPin);      // Stop the tone
  
  if (DEBUG_MODE) {
    Serial.println("Button sound played!");
  }
}