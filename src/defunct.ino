#include <LiquidCrystal.h>
#include <RTClib.h>
#include <DHT.h>
#include <Stepper.h>

// Define pin assignments
const int buttonInterruptPin = 3;   // Button connected to interrupt pin
const int relay1Pin = 53;           // Fan Relay In
const int relay2Pin = 52;           // Pump Relay In
const int buttonLeftPin = 12;       // STPRR
const int buttonRightPin = 13;      // STPRL
const int led1Pin = 10;             // LED G
const int led2Pin = 9;              // LED YL
const int led3Pin = 8;              // LED R

// Stepper motor pins and configuration
const int stepsPerRevolution = 2048;  // 28BYJ-48 stepper has 2048 steps per revolution
const int stepper1Pin = 43;           // IN1
const int stepper2Pin = 45;           // IN2
const int stepper3Pin = 47;           // IN3
const int stepper4Pin = 49;           // IN4

// Initialize stepper library
Stepper myStepper(stepsPerRevolution, stepper1Pin, stepper3Pin, stepper2Pin, stepper4Pin);

// Variables to track butw states
volatile bool interruptButtonPressed = false;
bool lastLeftButtonState = HIGH;
bool lastRightButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // Debounce time in milliseconds

// States the system will be in
enum SystemState {
  DISABLED,
  IDLE,
  ERROR,
  RUNNING,
};

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("Component Test Program");
  
  // Set pin modes for buttons (using external pull-down resistors)
  pinMode(buttonInterruptPin, INPUT);
  pinMode(buttonLeftPin, INPUT);
  pinMode(buttonRightPin, INPUT);
  
  // Set pin modes for relays and LEDs
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  
  // Initial states - relays and LEDs off
  digitalWrite(relay1Pin, LOW);
  digitalWrite(relay2Pin, LOW);
  digitalWrite(led1Pin, LOW);
  digitalWrite(led2Pin, LOW);
  digitalWrite(led3Pin, LOW);
  
  // Attach interrupt for the interrupt button
  attachInterrupt(digitalPinToInterrupt(buttonInterruptPin), handleInterrupt, RISING);
  
  // Set stepper speed
  myStepper.setSpeed(25);  // is slow
  
  Serial.println("Setup complete. Ready for testing.");
}

void loop() {
  // Check interrupt button
  if (interruptButtonPressed) {
    Serial.println("Interrupt button pressed - Toggling relays");
    
    // Toggle relays
    static bool relaysOn = false;
    relaysOn = !relaysOn;
    digitalWrite(relay1Pin, relaysOn);
    digitalWrite(relay2Pin, relaysOn);
    
    // Green LED (led1Pin) indicates relay state
    digitalWrite(led1Pin, relaysOn);
    
    // Reset flag
    interruptButtonPressed = false;
    
    // Small delay to avoid switch bounce even with caps
    delay(50);
  }
  
  // Handle stepper motor buttons with simple direct reading
  // Read left and right button states
  int leftReading = digitalRead(buttonLeftPin);
  int rightReading = digitalRead(buttonRightPin);
  
  // Debug button states
  static int lastLeftPrinted = -1;
  static int lastRightPrinted = -1;
  if (leftReading != lastLeftPrinted) {
    Serial.print("Left button state: ");
    Serial.println(leftReading);
    lastLeftPrinted = leftReading;
  }
  if (rightReading != lastRightPrinted) {
    Serial.print("Right button state: ");
    Serial.println(rightReading);
    lastRightPrinted = rightReading;
  }
  
  // Simple direct control for debugging
  if (leftReading == HIGH) {
    Serial.println("Moving stepper left");
    digitalWrite(led2Pin, HIGH);  // Yellow LED on
    myStepper.step(10);  // Small movement for testing
    digitalWrite(led2Pin, LOW);   // Yellow LED off
  }
  
  if (rightReading == HIGH) {
    Serial.println("Moving stepper right");
    digitalWrite(led3Pin, HIGH);  // Red LED on
    myStepper.step(-10);  // Small movement for testing
    digitalWrite(led3Pin, LOW);   // Red LED off
  }
  
  // Small delay
  delay(50);
}

// Interrupt service routine
void handleInterrupt() {
  interruptButtonPressed = true;
}