volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
volatile unsigned char *myTIFR1 =  (unsigned char *) 0x36;
volatile unsigned char *portDDRB = (unsigned char *) 0x24;
volatile unsigned char *portB =    (unsigned char *) 0x25;

//make sure to always use volatile keyword


void setup() 

  Serial.begin(9600);
}
void loop() 
{

}
//replace '??' With your value
void my_delay(unsigned int freq)
{
  // calc period
  double period = 1.0/double(freq);
  // 50% duty cycle
  double half_period = period/ 2.0f;
  // clock period def
  double clk_period = 0.0000000625;
  // calc ticks
  unsigned int ticks = ??;
  // stop the timer
  *myTCCR1B &= ??;
  // set the counts
  *myTCNT1 = (unsigned int) (65536 - ticks);

  * myTCCR1A = 0x0;
  // start the timer
  * myTCCR1B |= ??;
  // wait for overflow
  while((*myTIFR1 & 0x01)==0); 
  // stop the timer
  *myTCCR1B &= ??;   
  // reset TOV           
  *myTIFR1 |= ??;
}
