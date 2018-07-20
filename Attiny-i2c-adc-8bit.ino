/* attiny85 set up as i2c dual adc sensor */
/* Rev2  19/7/18 Bruce Woolmore;  use pin 6 (digital 1) as sink for voltage divider/comparators
/* I2C Slave address.You can have multiple sensors with different addresses */
#define I2C_SLAVE_ADDRESS 0x13
/* chip pin 2 = arduino pin 3 = adc pin A3 ,chip pin 3 = arduino pin 4 = adc pin A2 */ 
/* also use chip pin 6 = arduino pin 1  as sink */
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

#include <TinyWireS.h>    // Get this from https://github.com/rambo/TinyWire

volatile uint8_t i2c_regs[] =
{0xDE,0xAD,0xBE,0xEF,0xFE,0xED}; // init regs with known content for diags

// Tracks the current register pointer position
volatile byte reg_position;
const byte reg_size = sizeof(i2c_regs);

/**
 * This is called for each read request we receive, never put more than one byte of data (with TinyWireS.send) to the 
 * send-buffer when using this callback
 */
void requestEvent()
{  
    TinyWireS.send(i2c_regs[reg_position]);
    // Increment the reg position on each read, and loop back to zero
    reg_position++;
    if (reg_position >= reg_size)
    {
        reg_position = 0;
    }
}

/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) on the bus does
 * so be quick, set flags for long running tasks to be called from the mainloop instead of running them directly,
 */
void receiveEvent(uint8_t howMany)
{
    if (howMany > TWI_RX_BUFFER_SIZE)
    {
        // if insane number
        return;
    }

    reg_position = TinyWireS.receive();
    howMany--;
    if (!howMany)
    {
        // No data, so this write was only to set the buffer for next read
        return;
    }
    while(howMany--)
    {  // if any data received, it is the number of the adc to sample, put in reg 00
        i2c_regs[0] = TinyWireS.receive();
    }
}

void initADC()
{
  /* this function initialises the ADC;   uses 8-bit resolution;  8bit = approx 20mv resolution
 see datasheet for prescaler notes; for 8mhz, set adps 2/1/0 to 1/1/0 for /64
 set prescaler to 0/1/1 for mcu running at 1MHz (/8=125khz)
 set ADLAR to 1 to enable the Left-shift result (only bits ADC9..ADC2 are available)
 then, only reading ADCH is sufficient for 8-bit results (256 values)  
 adc selection; set Mux0 to 0 for adc2 1 for adc3  
*/

  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (0 << MUX3)  |     // MUX bit 3 not curr used
            (0 << MUX2)  |     // MUX bit 2 not curr used
            (1 << MUX1)  |     // MUX bit 1 set = ADC 2 or 3
            (0 << MUX0);       // MUX bit 0 set for ADC3 unset  for ADC2

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (0 << ADPS2) |     // set prescaler to 8, bit 2 
            (1 << ADPS1) |     // set prescaler to 8, bit 1 
            (1 << ADPS0);      // set prescaler to 8, bit 0  
}

uint8_t sample_adc(int channel) 
{
// 19/7/18 set voltage divider sink pin LOW
pinMode(1,OUTPUT);
digitalWrite(1,LOW);
 // select which adc to use (set ADMUX bit 0 on/off = adc3/2
 if(channel==3) { ADMUX |= (1<<0); }  // set last bit of ADMUX on
 else           { ADMUX &= ~(1<<0); }    // set last bit of ADMUX off
 
 // take 100x 8-bit samples and calculate a rolling average of the last 15 samples
 float voltage_fl=0;        // battery voltage rolling avg
 int sample_loop;
 for (sample_loop=50; sample_loop > 0 ; sample_loop --)
 {
   ADCSRA |= (1 << ADSC);         				// start ADC measurement
   while (ADCSRA & (1 << ADSC) ){}; 			// wait till conversion complete 
   voltage_fl = voltage_fl + ((ADCH - voltage_fl) / 15); // last 15 rolling average
 }
  uint8_t result = (uint8_t) voltage_fl+0.5; // round float to integer
  i2c_regs[4]=ADMUX; 		// save ADMUX settings for diag use
  i2c_regs[5]=ADCH;  		// save last adc reading for diag use
  digitalWrite(1,HIGH);   		// set sink pin HIGH to turn off voltage divider circuit
  return result;
}

void setup()
{
    initADC(); // set attiny adc operating mode registers
      
    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onReceive(receiveEvent);
    TinyWireS.onRequest(requestEvent);
}
  
void loop()
{
 TinyWireS_stop_check();
 if (i2c_regs[0]!=0xDE) // if an adc has been selected proceed
   {
   // if reg 0 set, return sample of specified adc in its i2c register 
   if (i2c_regs[0]==2) 
	  {
	  i2c_regs[2] = sample_adc(2);
    i2c_regs[1]=0xA2; // show adc 2 used
	  }
   else if (i2c_regs[0]==3)
	  {
    i2c_regs[3] = sample_adc(3);
    i2c_regs[1]=0xA3; // show adc3 used
	  }
   else
    {
    i2c_regs[0]=0xDE; // reset reg 0 = no adc sampling
    i2c_regs[1]=0xEE; // error flag
    }
   }   
}
