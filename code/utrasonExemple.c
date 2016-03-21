/*
* ****************************************************************************
* RP6 ROBOT SYSTEM - RP6 CONTROL M32 Examples
* ****************************************************************************
* Example: Afstandsmeting met de SRF05
*
* ****************************************************************************
*
*****************************************************************************/
// Includes:

#include "RP6ControlLib.h"       // The RP6 Control Library.
                        // Always needs to be included!

//#include "RP6I2CmasterTWI.h"   // I2C Master Library

#include "main.h"

/*****************************************************************************/
/*****************************************************************************/
// Include our new "RP6 Control I2C Master library":

//#include "RP6Control_I2CMasterLib.h"

/*****************************************************************************/


int main(void)
{
   initRP6Control(); 
   initLCD();
   showScreenLCD("SRF05 afstands","meter");
   mSleep(1000);
   setLEDs(0b0000);
   
   sound(100,40,64);
   sound(170,40,0);
   clearLCD();
   
   setup();
      
   unsigned int range;   
   

   while(true)
   {    
      
      startRange();         // Send a high on the trigger pin to start a ranging
      range = getEcho();      // Wait for the echo line to go high and then measure the length of this high
      displayData(range);
      
      startTimer(0xFFFF);      // Delay before taking another ranging
      waitForTimer();
            
   }
return 0;   
}   
   

   // Afstandsmeting
void startTimer(unsigned int time)
{
   OCR1A = time;         // Time set to count to
   TIFR = 0x10;         // Clear timer campare match flag
   TCNT1 = 0x00;          // Clear timer to zero
   TCCR1B = START_CLK;      // Start timer running 1:8 prescaler
}

void waitForTimer(void)
{
   while(!(TIFR&0x10));   // wait for timer to set compare match flag
   TCCR1B = STOP_CLK;      // Stop the timer
}


void setup(void)
{
   DDRD=0x20;         // Port D pin 5 ouput triggerpin SRF05
   TCCR1A = 0x00;         // Set timer up in CTC mode
   TCCR1B = 0x08;   
}

 
void startRange(void)
{
   DDRD=0x20;               // make D5 output
   PORTD = ( 1<<PORTD5 );      // Send trigger pin D5 high
   startTimer(0x0007);         // Wait around 10uS before sending it low again
   waitForTimer();
   PORTD = ( 0<<PORTD5 );      // Send trigger pin D5 low
}

unsigned int getEcho(void)
{
uint8_t range;               
   DDRD=0x00;               // Make pin D5 input
   while(!(PIND&0x20));      // Wait for echo pin D5 to go high,  this indicates the start of the incoming pulse
   TCNT1 = 0x00;             // Clear timer to zero
   TCCR1B = START_CLK_N;      // Start timer running 1:8 prescaler in normal mode
   while((PIND&0x20));      // Wait for echo pin D5 to go low signaling that the pulse has ended
   TCCR1B = STOP_CLK;         // Stop the timer and set back to CTC mode   
   range = TCNT1/116;         // Read back value in the timer counter register, this number divided by 116 will give us the range in CM
   return(range);
   
}

void displayData(unsigned int r)
{
   clearLCD();
   writeStringLCD("Afstand is ");
   writeIntegerLCD(r, DEC);
   writeStringLCD(" cm");
} 

//  Defines

#define START_CLK   0x0A            // Start clock with 1:8 prescaler CTC mode
#define START_CLK_N   0x02            // Start clock running 1:8 prescaler in normal mode
#define STOP_CLK   0x08            // Stop clock
//#define LCD03      0xC6            // Address of LCD03

// Prototypes

// main.c
void startRange(void);
unsigned int getEcho(void);
void displayData(unsigned int r);
void setup(void);                  // General chip setup
void startTimer(unsigned int time);      // Starts the timer running for a number of uS defined by value of time
void waitForTimer(void);            // Waits for the timer to stop
