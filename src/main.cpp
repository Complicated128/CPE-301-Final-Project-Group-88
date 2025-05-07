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
volatile unsigned char *ddra = (unsigned char*) 0x21;
volatile unsigned char *porta = (unsigned char*) 0x22;
volatile unsigned char *pina = (unsigned char*) 0x20;
/**
 * D52 (PB1), D53 (PB0), D12 (PB6), D13 (PB7), D10 (PB4)
 */
volatile unsigned char *ddrb = (unsigned char*) 0x24;
volatile unsigned char *portb = (unsigned char*) 0x25;
volatile unsigned char *pinb = (unsigned char*) 0x23;
/**
 * D37 (PC0), D35 (PC2), D33 (PC4)
 */
volatile unsigned char *ddrc = (unsigned char*) 0x27;
volatile unsigned char *portc = (unsigned char*) 0x28;
volatile unsigned char *pinc = (unsigned char*) 0x26;
/**
 * D20 (PD1), D21 (PD0)
 */
volatile unsigned char *ddrd = (unsigned char*) 0x2A;
volatile unsigned char *portd = (unsigned char*) 0x2B;
volatile unsigned char *pind = (unsigned char*) 0x29;
/**
 * D3 (PE5)
 */
volatile unsigned char *ddre = (unsigned char*) 0x2D;
volatile unsigned char *porte = (unsigned char*) 0x2E;
volatile unsigned char *pine = (unsigned char*) 0x2C;
/**
 * D54/A0 (PF0)
 */
volatile unsigned char *ddrf = (unsigned char*) 0x30;
volatile unsigned char *portf = (unsigned char*) 0x31;
volatile unsigned char *pinf = (unsigned char*) 0x2F;
/**
 * D39 (PG2)
 */
volatile unsigned char *ddrg = (unsigned char*) 0x33;
volatile unsigned char *portg = (unsigned char*) 0x34;
volatile unsigned char *ping = (unsigned char*) 0x32;
/**
 * D9 (PH6), D8 (PH5)
 */
volatile unsigned char *ddrh = (unsigned char*) 0x101;
volatile unsigned char *porth = (unsigned char*) 0x102;
volatile unsigned char *pinh = (unsigned char*) 0x100;
/**
 * D43 (PL6), D45 (PL4), D47 (PL2), D49 (PL0)
 */
volatile unsigned char *ddrl = (unsigned char*) 0x10A;
volatile unsigned char *portl = (unsigned char*) 0x10B;
volatile unsigned char *pinl = (unsigned char*) 0x109;

//UART pointers
volatile unsigned char *myUCSR0A = (unsigned char*) 0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char*) 0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char*) 0x00C2;
volatile unsigned char *myUBRR0 = (unsigned char*) 0x00C4;
volatile unsigned char *myUDR0 = (unsigned char*) 0x00C6;

//Timer Pointers
volatile unsigned char *myTCCR1A = (unsigned char*) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char*) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char*) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char*) 0x6F;
volatile unsigned char *myTIFR1 = (unsigned char*) 0x36;
volatile unsigned int *myTCNT1 = (unsigned int*) 0x84;

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

// Initialize the liquid crystal display
LiquidCrystal lcd(29, 27, 33, 35, 37, 39);

void setup_timer_regs();
void U0Init(int); // serial port initialization
ISR(TIMER1_OVF_vect);
void printMessage(unsigned char[]);
void putChar(unsigned char); // smaller version of printMessage

// States the system will be in
enum SystemState {
  DISABLED,
  IDLE,
  ERROR,
  RUNNING,
};

void setup() {
  // put your setup code here, to run once:
  U0Init(9600);
  unsigned char msg[] = "Component Test Program";
  printMessage(msg);


  unsigned char msg[] = "Setup complete. Ready for testing.";
  printMessage(msg);
}
// Temperature Threshold = 10
// Water Level Threshold = 320
States state = IDLE; // Set the initial state to IDLE
void loop() {
  if (temp <= 10) {
    state = IDLE;
  } else if (temp > 10) {
    state = RUNNING;
  } else if (state == ERROR && 
  }
  // put your main code here, to run repeatedly:
  // switch (state) {
  //   case DISABLED:
  //     // Handle disabled state
      
  //     break;
  //   case IDLE:
  //     // Handle idle state
  //     if (interruptButtonPressed) {
  //       // Handle button press
  //       interruptButtonPressed = false;
  //     }
  //     break;
  //   case ERROR: 
  //     // Handle error state
  //     break;
  //   case RUNNING:
  //     // Handle running state
  //     break;
  //   default:
  //     break; 
  // }
}

void setup_timer_regs()
{
  
}

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

ISR(TIMER1_OVF_vect)
{
  // *myTCCR1B &= 0xF8;
  // *myTCNT1 = (unsigned int)(65535 - (unsigned long)(currentTicks));
  // *myTCCR1B |= 0x01;
  // if (currentTicks != 65535)
  // {
  //   *portb ^= 0x40;
  // }
}

void putChar(unsigned char U0pdata)
{
  while ((*myUCSR0A & TBE) == 0);
  *myUDR0 = U0pdata;
}

void printMessage(unsigned char msg[])
{
  for (int i = 0; msg[i] != '\0'; i++)
  {
    putChar(msg[i]);
  }
}