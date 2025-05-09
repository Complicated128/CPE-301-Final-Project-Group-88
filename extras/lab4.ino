volatile unsigned char *ddrb = (unsigned char *)0x24;
volatile unsigned char *portb = (unsigned char *)0x25;

volatile unsigned char *myTCCR1A = (unsigned char *)0x80;
volatile unsigned char *myTCCR1B = (unsigned char *)0x81;
volatile unsigned char *myTCCR1C = (unsigned char *)0x82;
volatile unsigned int *myTCNT1 = (unsigned int *)0x84;
volatile unsigned char *myTIFR1 = (unsigned char *)0x36;

void setup() {
  Serial.begin(9600);
  *ddrb |= (0x01 << 6);  // setting up output
}

unsigned char in_char;

void loop() {
  if (Serial.available()) {
    in_char = Serial.read();
  }
  Serial.write(in_char);
  switch (in_char) {
    case 'A':
      //440
      *portb |= (0x01 << 6);
      freqDelay(440, 0.5);
      *portb &= ~(0x01 << 6);
      Serial.write(in_char);
      break;
    case 'B':
      //494
      *portb |= (0x01 << 6);
      freqDelay(494, 0.5);
      *portb &= ~(0x01 << 6);
      Serial.write(in_char);
      break;
    case 'C':
      //523
      *portb |= (0x01 << 6);
      freqDelay(523, 0.5);
      *portb &= ~(0x01 << 6);
      Serial.write(in_char);
      break;
    case 'D':
      //587
      *portb |= (0x01 << 6);
      freqDelay(587, 0.5);
      *portb &= ~(0x01 << 6);
      Serial.write(in_char);
      break;
    case 'E':
      //659
      *portb |= (0x01 << 6);
      freqDelay(659, 0.5);
      *portb &= ~(0x01 << 6);
      Serial.write(in_char);
      break;
    case 'F':
      //698
      *portb |= (0x01 << 6);
      freqDelay(698, 0.5);
      *portb &= ~(0x01 << 6);
      Serial.write(in_char);
      break;
    case 'G':
      //784
      *portb |= (0x01 << 6);
      freqDelay(784, 0.5);
      *portb &= ~(0x01 << 6);
      Serial.write(in_char);
      break;
    case 'q':
      Serial.write(in_char);
      break;
  }
}

double const CLK_PERIOD = 0.0000000625;

void freqDelay(unsigned int freq, double dutyCycle) {
  double period_duty = 1.0 / (dutyCycle * (double)freq);
  unsigned int ticks = period_duty / CLK_PERIOD;
  *myTCCR1B &= 0xF9;
  *myTCNT1 = (unsigned int)(65536 - ticks);
  *myTCCR1A = 0x0;
  // start the timer
  *myTCCR1B |= 0x01;
  // wait for overflow
  while ((*myTIFR1 & 0x01) == 0)
    ;
  // stop the timer
  *myTCCR1B &= 0xF9;
  // reset TOV
  *myTIFR1 |= 0x01;
}
