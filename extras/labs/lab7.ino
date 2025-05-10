#define RDA 0x80
#define TBE 0x20  

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
 
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

void setup() 
{
  // setup the UART
  U0init(9600);
  // setup the ADC
  adc_init();
}
void loop() 
{
  // read the water sensor value by calling adc_read() and check the threshold to display the message accordingly
  //if the value is less than the threshold display the value on the Serial monitor
  unsigned int threshold = 290;
  unsigned int sensorVal = adc_read(0);
  if (sensorVal < threshold)
  {
    U0putchar('V');
    U0putchar(':');
    U0putchar(sensorVal / 100 + '0');
    U0putchar((sensorVal / 10) % 10 + '0');
    U0putchar(sensorVal % 10 + '0');
    U0putchar('\n');
  }
  else
  {
    const char* msg = "Water Level: High";
    for(int i = 0; msg[i] != '\0'; i++)
    {
      U0putchar(msg[i]);
    }
    U0putchar('\n');
  }
  //if the value is over the threshold display "Water Level: High" message on the Serial monitor.

  //Use a threshold value that works for you with your sensor. There is no fixed value as sensor's sensitivity can differ.
}
void adc_init() //write your code after each commented line and follow the instruction 
{
  // setup the A register
 // set bit   7 to 1 to enable the ADC
  *my_ADCSRA |= 0x80;
 // clear bit 6 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0xBF;
 // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0xEF;
 // clear bit 0-2 to 0 to set prescaler selection to slow reading
  *my_ADCSRA &= 0xF8;
  // setup the B register
// clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0xF0;
 // clear bit 2-0 to 0 to set free running mode

  // setup the MUX Register
 // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX = 0x40;
// set bit 6 to 1 for AVCC analog reference

  // clear bit 5 to 0 for right adjust result

 // clear bit 4-0 to 0 to reset the channel and gain bits

}
unsigned int adc_read(unsigned char adc_channel_num) //work with channel 0
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX &= 0xE0;

  // clear the channel selection bits (MUX 5) hint: it's not in the ADMUX register
  *my_ADCSRB &= 0xDF;
 
  // set the channel selection bits for channel 0
  *my_ADMUX |= (adc_channel_num & 0x1F);

  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;

  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register and format the data based on right justification (check the lecture slide)
  
  unsigned int val = *my_ADC_DATA;
  return val;
}

void U0init(int U0baud)
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}
unsigned char U0getchar()
{
  return *myUDR0;
}
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}
