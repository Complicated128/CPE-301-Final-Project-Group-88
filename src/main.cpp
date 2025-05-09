#include <Arduino.h>
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <DHT.h>
#include <Stepper.h>

#define RDA 0x80
#define TBE 0x20

// Define pin assignments
#define INTERRUPT_PIN 3
#define FAN_RELAY_PIN 53
#define PUMP_RELAY_PIN 52
#define RIGHT_STEPPER_PIN 12
#define LEFT_STEPPER_PIN 13
#define LED_PIN1 = 10
#define LED_PIN2 = 9
#define LED_PIN3 = 8
#define LED_PIN4 = 7

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

// Stepper motor pins and configuration
#define STEPS_PER_REV 2048
#define STEPPER_PIN1 43
#define STEPPER_PIN2 45
#define STEPPER_PIN3 47
#define STEPPER_PIN4 49

// Initialize stepper library
Stepper myStepper(STEPS_PER_REV, STEPPER_PIN1, STEPPER_PIN3, STEPPER_PIN2, STEPPER_PIN4);

// Initialize the liquid crystal display
#define LC_RS 29
#define LC_EN 27
#define LC_D4 33
#define LC_D5 35
#define LC_D6 37
#define LC_D7 39
LiquidCrystal lcd(LC_RS, LC_EN, LC_D4, LC_D5, LC_D6, LC_D7);

// Initialize RTC
RTC_DS1307 rtc;

// Initialize DHT
DHT dht(25, DHT11);

// States the system will be in
enum SystemState
{
   DISABLED,
   IDLE,
   ERROR,
   RUNNING,
};

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
void msTimerDelay(unsigned int);

// Global variables to determine button hold
volatile bool buttonPressed = false;
volatile unsigned long startTime = 0;
volatile unsigned long pressDuration = 0;
volatile bool buttonHold = false;

// Global variables
SystemState currentState = IDLE;
SystemState previousState = IDLE;
volatile bool interruptBtn = false;
volatile bool fanOn = false;
volatile bool displayTH = false;
volatile bool waterMonitor = false;
volatile bool needClear = false;
volatile unsigned int waterThreshold = 320; // value to change
volatile unsigned int tempThreshold = 10;   // value to change

void setup()
{
   U0Init(9600);                                   // Start the UART serial communication
   adc_init();                                     // Setup the ADC
   rtc.begin();                                    // Real Time Clock setup
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Setuup temp and humidity sensor
   dht.begin();                                    // start temp and humidity
   lcd.begin(16, 2);                               // Setup the LCD

   printMessage((unsigned char *)"Component Test Program\0");

   // pinMode(buttonInterruptPin, INPUT); // PE5
   *ddre &= ~(0x01 << 5);
   // pinMode(buttonRightPin, INPUT); // PB6
   *ddrb &= ~(0x01 << 6);
   // pinMode(buttonLeftPin, INPUT); // PB7
   *ddrb &= ~(0x01 << 7);

   // Set pin modes for relays and LEDs
   // pinMode(relay1Pin, OUTPUT); //D53 = PB0
   // pinMode(relay2Pin, OUTPUT); //D52 = PB1
   // pinMode(led1Pin, OUTPUT); //D10 = PB4
   // pinMode(led2Pin, OUTPUT); //D9 = PH6
   // pinMode(led3Pin, OUTPUT); //D8 = PH5
   *ddrb |= (0x01 << 0);
   *ddrb |= (0x01 << 1);
   *ddrb |= (0x01 << 4);
   *ddrh |= (0x01 << 6);
   *ddrh |= (0x01 << 5);

   // Initial states - relays and LEDs off
   // digitalWrite(relay1Pin, LOW);
   // digitalWrite(relay2Pin, LOW);
   // digitalWrite(led1Pin, LOW);
   // digitalWrite(led2Pin, LOW);
   // digitalWrite(led3Pin, LOW);
   // digitalWrite(led4Pin, LOW);
   *portb &= ~(0x01 << 0);
   *portb &= ~(0x01 << 1);
   *portb &= ~(0x01 << 4);
   *porth &= ~(0x01 << 6);
   *porth &= ~(0x01 << 5);
   *porth &= ~(0x01 << 4);

   // interrupt setup
   attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInterrupt, RISING);

   myStepper.setSpeed(25);

   printMessage((unsigned char *)"Setup complete. Ready for testing.\0");
}

unsigned long prevMillis = 0;
const unsigned long interval = 60000; // 1 min

void loop()
{
   if (interruptBtn)
   {
      if (currentState == DISABLED)
      {
         printMessage((unsigned char *)"Interrupt button pressed (starting)\0");
         currentState = IDLE;
      }
      else
      {
         printMessage((unsigned char *)"Interrupt button pressed (disabling)\0");
         currentState = DISABLED;
      }
      if (buttonHold && currentState == ERROR)
      {
         printMessage((unsigned char *)"Interrupt button held (resetting)\0");
         setup(); // not sure if its required
         currentState = IDLE;
      }
      interruptBtn = false;
   }
   if (currentState == DISABLED)
   {
      return;
   }
   // Checks what currentState the system needs to be in
   unsigned int waterVal = adc_read(0);
   unsigned int tempVal = dht.readTemperature();
   stateCheck(waterVal, tempVal);
   if (currentState != previousState)
   {
      displayTimeStamp();
      previousState = currentState;
   }
   // Change the currentState of the system
   switch (currentState)
   {
   case DISABLED:
      // Handle disabled currentState
      fanOn = false;
      displayTH = false;
      waterMonitor = false;
      // Turn on yellow LED
      *portb &= ~(0x01 << 4);
      *porth |= (0x01 << 6);
      *porth &= ~(0x01 << 5);
      *porth &= ~(0x01 << 4);
      // Turn off fans and pump
      *portb &= ~(0x01 << 0);
      *portb &= ~(0x01 << 1);
      break;

   case IDLE:
      // Handle idle currentState
      fanOn = false;
      displayTH = true;
      waterMonitor = true;
      // Turn on green LED
      *portb |= (0x01 << 4);
      *porth &= ~(0x01 << 6);
      *porth &= ~(0x01 << 5);
      *porth &= ~(0x01 << 4);
      // Turn off fans and pump
      *portb &= ~(0x01 << 0);
      *portb &= ~(0x01 << 1);
      break;

   case ERROR:
      // Handle error currentState
      lcd.clear();
      lcd.print("Error: Low Water Level");
      fanOn = false;
      displayTH = true;
      waterMonitor = true;
      // Turn on red LED
      *portb &= ~(0x01 << 4);
      *porth &= ~(0x01 << 6);
      *porth |= (0x01 << 5);
      *porth &= ~(0x01 << 4);
      // Turn off fans and pump
      *portb &= ~(0x01 << 0);
      *portb &= ~(0x01 << 1);
      break;

   case RUNNING:
      // Handle running currentState
      fanOn = true;
      displayTH = true;
      waterMonitor = true;
      // Turn on blue LED
      *portb &= ~(0x01 << 4);
      *porth &= ~(0x01 << 6);
      *porth &= ~(0x01 << 5);
      *porth |= (0x01 << 4);
      // Turn on fans and pump
      *portb |= (0x01 << 0);
      *portb |= (0x01 << 1);
      break;

   default:
      break;
   }
   // Display temperature and humidity on LCD
   if (displayTH && millis() - prevMillis >= interval)
   {
      // TODO: fix the display function.
      displayTempAndHum(tempVal, (unsigned int)dht.readHumidity());
      prevMillis = millis();
   }

   // Only allows the stepper motor to be controlled when the system is not in a DISABLED state
   if (currentState != DISABLED)
   {
      // Check if the right stepper button is pressed
      if (*pinb & (0x01 << 6))
      {
         // Move the stepper motor clockwise
         myStepper.step(-10);
         displayTimeStamp();
      }
      // Check if the left stepper button is pressed
      else if (*pinb & (0x01 << 7))
      {
         // Move the stepper motor counterclockwise
         myStepper.step(10);
         displayTimeStamp();
      }
   }
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
   while ((*myADCSRA & 0x40) != 0);
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

//ISR
void handleInterrupt()
{
   if (!buttonPressed) // will be true, button pressed
   {
      buttonPressed = true;
      startTime = millis();
   }
   else // button released
   {
      buttonPressed = false;
      pressDuration = millis() - startTime;
      if (pressDuration >= 3000)
      {
         buttonHold = true;
      }
      else
      {
         buttonHold = false;
      }
      interruptBtn = true;
   }
}

void displayTimeStamp()
{
   DateTime now = rtc.now();
   String date;
   char format[] = "YYYY-MM-DD hh:mm:ss";
   date = now.toString(format);
   printMessage((unsigned char *)format);
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
   lcd.print("Temp: " + (String)temp + char(248) + "C");
   lcd.setCursor(0, 1);
   lcd.print("Hum: " + (String)hum + "%");
}

void stateCheck(unsigned int waterLevel, unsigned int tempLevel)
{
   if (tempLevel > tempThreshold) // temp higher than expected, run
   {
      currentState = RUNNING;
   }
   else if (tempLevel <= tempThreshold) // temp lower than expected, idle
   {
      currentState = IDLE;
   }
   if (waterLevel <= waterThreshold) // water lower than expected, error
   {
      // both cases of running -> error and idle -> error have similar cases
      // running -> error requires waterLevel < waterThreshold
      // idle -> error requires waterLevel <= waterThreshold
      currentState = ERROR;
   }
}