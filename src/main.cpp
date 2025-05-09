#include <Arduino.h>
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <DHT.h>
#include <Stepper.h>

#define RDA 0x80
#define TBE 0x20

// setting up vars for ports
/**
 * D29 (PA7), D27 (PA5), D25 (PA3)
 */
volatile unsigned char *ddra = (unsigned char *)0x21;
volatile unsigned char *porta = (unsigned char *)0x22;
volatile unsigned char *pina = (unsigned char *)0x20;
/**
 * D52 (PB1), D53 (PB0), D12 (PB6), D13 (PB7), D10 (PB4)
 */
volatile unsigned char *ddrb = (unsigned char *)0x24;
volatile unsigned char *portb = (unsigned char *)0x25;
volatile unsigned char *pinb = (unsigned char *)0x23;
/**
 * D37 (PC0), D35 (PC2), D33 (PC4)
 */
volatile unsigned char *ddrc = (unsigned char *)0x27;
volatile unsigned char *portc = (unsigned char *)0x28;
volatile unsigned char *pinc = (unsigned char *)0x26;
/**
 * D20 (PD1), D21 (PD0)
 */
volatile unsigned char *ddrd = (unsigned char *)0x2A;
volatile unsigned char *portd = (unsigned char *)0x2B;
volatile unsigned char *pind = (unsigned char *)0x29;
/**
 * D3 (PE5)
 */
volatile unsigned char *ddre = (unsigned char *)0x2D;
volatile unsigned char *porte = (unsigned char *)0x2E;
volatile unsigned char *pine = (unsigned char *)0x2C;
/**
 * D54/A0 (PF0)
 */
// ANALOG
volatile unsigned char *ddrf = (unsigned char *)0x30;
volatile unsigned char *portf = (unsigned char *)0x31;
volatile unsigned char *pinf = (unsigned char *)0x2F;
/**
 * D39 (PG2)
 */
volatile unsigned char *ddrg = (unsigned char *)0x33;
volatile unsigned char *portg = (unsigned char *)0x34;
volatile unsigned char *ping = (unsigned char *)0x32;
/**
 * D9 (PH6), D8 (PH5), D7 (PH4)
 */
volatile unsigned char *ddrh = (unsigned char *)0x101;
volatile unsigned char *porth = (unsigned char *)0x102;
volatile unsigned char *pinh = (unsigned char *)0x100;
/**
 * D43 (PL6), D45 (PL4), D47 (PL2), D49 (PL0)
 */
volatile unsigned char *ddrl = (unsigned char *)0x10A;
volatile unsigned char *portl = (unsigned char *)0x10B;
volatile unsigned char *pinl = (unsigned char *)0x109;

// UART pointers
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned char *myUBRR0 = (unsigned char *)0x00C4;
volatile unsigned char *myUDR0 = (unsigned char *)0x00C6;

// Timer Pointers
volatile unsigned char *myTCCR1A = (unsigned char *)0x80;
volatile unsigned char *myTCCR1B = (unsigned char *)0x81;
volatile unsigned char *myTCCR1C = (unsigned char *)0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *)0x6F;
volatile unsigned char *myTIFR1 = (unsigned char *)0x36;
volatile unsigned int *myTCNT1 = (unsigned int *)0x84;

// Water sensor registers
volatile unsigned char *myADMUX = (unsigned char *)0x7C;
volatile unsigned char *myADCSRB = (unsigned char *)0x7B;
volatile unsigned char *myADCSRA = (unsigned char *)0x7A;
volatile unsigned int *myADC_DATA = (unsigned int *)0x78;

// Define pin assignments
const int buttonInterruptPin = 3; // Button connected to interrupt pin
const int relay1Pin = 53;         // Fan Relay In
const int relay2Pin = 52;         // Pump Relay In
const int buttonLeftPin = 12;     // STPRR
const int buttonRightPin = 13;    // STPRL
const int led1Pin = 10;           // LED G
const int led2Pin = 9;            // LED YL
const int led3Pin = 8;            // LED R
const int led4Pin = 7;            // LED B

// Stepper motor pins and configuration
const int stepsPerRevolution = 2048; // 28BYJ-48 stepper has 2048 steps per revolution
const int stepper1Pin = 43;          // IN1
const int stepper2Pin = 45;          // IN2
const int stepper3Pin = 47;          // IN3
const int stepper4Pin = 49;          // IN4

// Initialize stepper library
Stepper myStepper(stepsPerRevolution, stepper1Pin, stepper3Pin, stepper2Pin, stepper4Pin);

// Initialize the liquid crystal display
const int RS = 29, EN = 27, D4 = 33, D5 = 35, D6 = 37, D7 = 39;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

// Initialize RTC
RTC_DS1307 rtc;

// Initialize DHT
DHT dht(25, DHT11);

// Function prototypes
void U0Init(int); // serial port initialization
void adc_init();
unsigned int adc_read(unsigned char);
void printMessage(unsigned char[]); // print strings
void putChar(unsigned char);        // smaller version of printMessage
void handleInterrupt();
void displayTimeStamp();
void displayTempAndHum(unsigned int, unsigned int);
void stateCheck(unsigned int, unsigned int);

// States the system will be in
enum SystemState
{
   DISABLED,
   IDLE,
   ERROR,
   RUNNING,
};

// Global variables
SystemState currentState = DISABLED;
SystemState previousState = DISABLED;
bool interruptBtn = false;
bool fanOn = false;
bool displayTH = false;
bool stepperState = false;
bool waterMonitor = false;
unsigned int waterThreshold = 320; // value to change
unsigned int tempThreshold = 10;
// Temperature Threshold = 10
// Water Level Threshold = 320

void setup()
{
   // Start the UART serial communication
   U0Init(9600);
   // Setup the ADC
   adc_init();
   // Real Time Clock setup
   rtc.begin();
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   // Setuup temp and humidity sensor
   dht.begin();
   // Setup the LCD
   lcd.begin(16, 2);

   printMessage((unsigned char *)"Component Test Program\0");

   // pinMode(buttonInterruptPin, INPUT); // PE5
   *ddre &= ~(0x01 << 5);
   // pinMode(buttonLeftPin, INPUT); // PB6
   *ddrb &= ~(0x01 << 6);
   // pinMode(buttonRightPin, INPUT); // PB7
   *ddrb &= ~(0x01 << 7);

   // Set pin modes for relays and LEDs
   // pinMode(relay1Pin, OUTPUT); //D53 = PB0
   *ddrb |= (0x01 << 0);
   // pinMode(relay2Pin, OUTPUT); //D52 = PB1
   *ddrb |= (0x01 << 1);
   // pinMode(led1Pin, OUTPUT); //D10 = PB4
   *ddrb |= (0x01 << 4);
   // pinMode(led2Pin, OUTPUT); //D9 = PH6
   *ddrh |= (0x01 << 6);
   // pinMode(led3Pin, OUTPUT); //D8 = PH5
   *ddrh |= (0x01 << 5);

   // Initial states - relays and LEDs off
   // digitalWrite(relay1Pin, LOW);
   *portb &= ~(0x01 << 0);
   // digitalWrite(relay2Pin, LOW);
   *portb &= (0x01 << 1);
   // digitalWrite(led1Pin, LOW);
   *portb &= (0x01 << 4);
   // digitalWrite(led2Pin, LOW);
   *porth &= (0x01 << 6);
   // digitalWrite(led3Pin, LOW);
   *porth &= (0x01 << 5);
   // digitalWrite(led4Pin, LOW);
   *porth &= (0x01 << 4);

   // interrupt setup
   attachInterrupt(digitalPinToInterrupt(buttonInterruptPin), handleInterrupt, RISING);

   myStepper.setSpeed(25);

   printMessage((unsigned char *)"Setup complete. Ready for testing.\0");
}

void loop()
{
   unsigned int waterVal = adc_read(0);
   unsigned int tempVal = dht.readTemperature();
   if (interruptBtn)
   {
      printMessage((unsigned char *)"Interrupt button pressed- toggling relays\0");
      static bool relaysOn = false;
      relaysOn = !relaysOn;
      // digitalWrite(relay1Pin, relaysOn);
      *portb |= (0x01 << 0);
      // digitalWrite(relay2Pin, relaysOn);
      *portb |= (0x01 << 2);
      // Green LED (led1Pin) indicates relay currentState
      // digitalWrite(led1Pin, relaysOn);
      *portb |= (0x01 << 4);
   }

   // Checks what currentState the system needs to be in
   stateCheck(waterVal, tempVal);
   // Change the currentState of the system
   switch (currentState)
   {
   case DISABLED:
      // Handle disabled currentState
      fanOn = false;
      displayTH = false;
      stepperState = true;
      waterMonitor = false;
      // Turn on yellow LED
      *portb &= (0x01 << 4);
      *porth |= (0x01 << 6);
      *porth &= (0x01 << 5);
      *porth &= (0x01 << 4);
      break;
   case IDLE:
      // Handle idle currentState
      fanOn = false;
      displayTH = true;
      stepperState = true;
      waterMonitor = true;
      // Turn on green LED
      *portb |= (0x01 << 4);
      *porth &= (0x01 << 6);
      *porth &= (0x01 << 5);
      *porth &= (0x01 << 4);
      break;
   case ERROR:
      // Handle error currentState
      lcd.clear();
      lcd.print("Error: Low Water Level");
      fanOn = false;
      displayTH = true;
      stepperState = false;
      waterMonitor = true;
      // Turn on red LED
      *portb &= (0x01 << 4);
      *porth &= (0x01 << 6);
      *porth |= (0x01 << 5);
      *porth &= (0x01 << 4);
      break;
   case RUNNING:
      // Handle running currentState
      fanOn = true;
      displayTH = true;
      stepperState = true;
      waterMonitor = true;
      // Turn on blue LED
      *portb &= (0x01 << 4);
      *porth &= (0x01 << 6);
      *porth &= (0x01 << 5);
      *porth |= (0x01 << 4);
      break;
   default:
      break;
   }
   previousState = currentState;
}

/* Serial port initialization
 * U0baud: baud rate
 * FCPU: CPU frequency
 * tbaud: baud rate timer value
 */
void U0Init(int U0baud)
{
   unsigned long FCPU = 16000000;
   unsigned int tbaud = (FCPU / 16 / U0baud - 1);
   // same as (FCPU / (16 * U0baud)) - 1
   *myUCSR0A = 0x20;
   *myUCSR0B = 0x18;
   *myUCSR0C = 0x06;
   *myUBRR0 = tbaud;
}

/* ADC initialization
 * ADC prescaler = 64
 * ADC reference voltage = AVcc
 * ADC channel = 0
 * ADC interrupt = disabled
 * ADC trigger = disabled
 * ADC enable = enabled
 */
void adc_init()
{
   *myADCSRA |= 0x80; // enable ADC
   *myADCSRA &= 0xBF; // disable ADC trigger
   *myADCSRA &= 0xEF; // disable ADC interrupt
   *myADCSRA &= 0xF8; // set prescaler to 64
   *myADCSRB &= 0xF0; // clear gain bits
   *myADMUX = 0x40;   // set reference voltage to AVcc
}

unsigned int adc_read(unsigned char adc_num)
{
   *myADMUX &= 0xE0;             // clear MUX4:0
   *myADCSRB &= 0xDF;            // clear MUX5
   *myADMUX |= (adc_num & 0x1F); // channel 0 selection
   *myADCSRA |= 0x40;            // conversion
   while ((*myADCSRA & 0x40) != 0)
      ;
   unsigned int val = *myADC_DATA;
   return val;
}

void printMessage(unsigned char msg[])
{
   for (int i = 0; msg[i] != '\0'; i++)
   {
      putChar(msg[i]);
   }
}

void putChar(unsigned char U0pdata)
{
   while (!(*myUCSR0A & TBE))
      ;
   *myUDR0 = U0pdata;
}

void handleInterrupt()
{
   interruptBtn = true;
}

void displayTimeStamp()
{
   DateTime now = rtc.now();
   String date;
   date = now.toString("YYYY-MM-DD hh:mm:ss");
   // TODO: LCD
}

void displayTempAndHum(unsigned int temp, unsigned int hum)
{
   if (needClear)
   {
      lcd.clear();
      needClear = false;
   }
   lcd.setCursor(0, 0);
   lcd.print("Temp: " + (String)temp + char(223) + "C");
   lcd.setCursor(0, 1);
   lcd.print("Humidity: " + (String)hum);
}

void stateCheck(unsigned int waterLevel, unsigned int tempLevel)
{
   if (tempLevel > tempThreshold)
   {
      currentState = RUNNING;
   }
   else if (tempLevel <= tempThreshold)
   {
      currentState = IDLE;
   }
   else if (waterLevel <= waterThreshold)
   {
      currentState = ERROR;
   }
}