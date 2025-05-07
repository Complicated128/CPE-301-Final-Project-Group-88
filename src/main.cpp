#include <Arduino.h>
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <DHT.h>
#include <Stepper.h>

#define RDA 0x80
#define TBE 0x20

// setting up vars for ports
volatile unsigned char *ddra = (unsigned char*) 0x21;
volatile unsigned char *porta = (unsigned char*) 0x22;
volatile unsigned char *pina = (unsigned char*) 0x20;

volatile unsigned char *ddrb = (unsigned char*) 0x24;
volatile unsigned char *portb = (unsigned char*) 0x25;
volatile unsigned char *pinb = (unsigned char*) 0x23;

volatile unsigned char *ddrc = (unsigned char*) 0x27;
volatile unsigned char *portc = (unsigned char*) 0x28;
volatile unsigned char *pinc = (unsigned char*) 0x26;

volatile unsigned char *ddre = (unsigned char*) 0x2D;
volatile unsigned char *porte = (unsigned char*) 0x2E;
volatile unsigned char *pine = (unsigned char*) 0x2C;

volatile unsigned char *ddrh = (unsigned char*) 0x33;
volatile unsigned char *porth = (unsigned char*) 0x34;
volatile unsigned char *pinh = (unsigned char*) 0x32;

volatile unsigned char *ddrh = (unsigned char*) 0x101;
volatile unsigned char *porth = (unsigned char*) 0x102;
volatile unsigned char *pinh = (unsigned char*) 0x100;

volatile unsigned char *ddrl = (unsigned char*) 0x10A;
volatile unsigned char *portl = (unsigned char*) 0x10B;
volatile unsigned char *pinl = (unsigned char*) 0x109;



// Stepper motor pins and configuration
const int stepsPerRevolution = 2048;  // 28BYJ-48 stepper has 2048 steps per revolution
const int stepper1Pin = 43;           // IN1
const int stepper2Pin = 45;           // IN2
const int stepper3Pin = 47;           // IN3
const int stepper4Pin = 49;           // IN4
// Initialize stepper library
Stepper myStepper(stepsPerRevolution, stepper1Pin, stepper3Pin, stepper2Pin, stepper4Pin);


// Variables to track button states
volatile bool interruptButtonPressed = false;
bool lastLeftButtonState = HIGH;
bool lastRightButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // Debounce time in milliseconds

void setup_timer_regs();
void U0Init(int);
ISR(TIMER1_OVF_vect);
unsigned char kbhit();

void setup() {
  // put your setup code here, to run once:
  U0Init;
}

void loop() {
  // put your main code here, to run repeatedly:

}

void setup_timer_regs()
{
  
}

void U0Init(int U0baud)
{

}

ISR(TIMER1_OVF_vect)
{

}

unsigned char kbhit()
{

}