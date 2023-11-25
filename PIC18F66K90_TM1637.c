// *****************************************************************************
// PIC 18F66K90 TM1637/keypad test code
//
// Test code that counts seconds on a TM1637 display
// A development version of my TM1637 library code is used here which
// introduces keyboard scanning of a 2x8 matrix via function keyscan()
// If a key is pressed the count display is replaced by the key number
//
// Hardware:
// TM1637 6 digit display with 2x8 keyboard as per the TM1637 datasheet. No caps on data/clk lines!
// PIC 18F66K90 with 5V power,16 MHz crystal,can use x4 PLL, ENABLED, *cannot* use XTAL>25MHz this chip,16 MHz max for PLL
// Max clock speed set up, 64 MHz, 16 MIPS
// Standard PicKit3 ICSP connections, MCLR to +5V via 10K resistor
// RG3 pin 6 LED1 (red) + 2k resistor   (note 2mA limit this port)
// RG4 pin 8 LED2 (green) + 2k resistor
//
// Demo code only no warranty of any kind
//
// Author: Steve Williams 14/10/2023
// *****************************************************************************


// PIC18F66K90 Configuration Bit Settings:

// CONFIG1L
#pragma config RETEN = ON       // VREG Sleep Enable bit (Enabled)
#pragma config INTOSCSEL = HIGH // LF-INTOSC Low-power Enable bit (LF-INTOSC in High-power mode during Sleep)
#pragma config SOSCSEL = HIGH   // SOSC Power Selection and mode Configuration bits (High Power SOSC circuit selected)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config FOSC = HS1       // Oscillator (HS oscillator (Medium power, 4 MHz - 16 MHz))  // Note 16MHzXTAL
#pragma config PLLCFG = ON      // PLL x4 Enable bit (Enabled, 64 MHz system clock)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2L
#pragma config PWRTEN = OFF     // Power Up Timer (Disabled)
#pragma config BOREN = OFF      // Brown Out Detect (Disabled in hardware, SBOREN disabled)
#pragma config BORV = 3         // 1 Brown-out Reset Voltage bits (1.8V)
#pragma config BORPWR = ZPBORMV // BORMV Power level (ZPBORMV instead of BORMV is selected)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer (WDT disabled in hardware; SWDTEN bit disabled)
#pragma config WDTPS = 1048576  // Watchdog Postscaler (1:1048576)

// CONFIG3L
#pragma config RTCOSC = SOSCREF // RTCC Clock Select (RTCC uses SOSC)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 Mux (RC1)
#pragma config MSSPMSK = MSK7   // MSSP address masking (7 Bit address masking mode)
#pragma config MCLRE = ON       // Master Clear Enable (MCLR Enabled, RG5 Disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Overflow Reset (Enabled)
#pragma config BBSIZ = BB2K     // Boot Block Size (2K word Boot Block size)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protect 00800-03FFF (Disabled)
#pragma config CP1 = OFF        // Code Protect 04000-07FFF (Disabled)
#pragma config CP2 = OFF        // Code Protect 08000-0BFFF (Disabled)
#pragma config CP3 = OFF        // Code Protect 0C000-0FFFF (Disabled)

// CONFIG5H
#pragma config CPB = OFF        // Code Protect Boot (Disabled)
#pragma config CPD = OFF        // Data EE Read Protect (Disabled)

// CONFIG6L
#pragma config WRT0 = OFF       // Table Write Protect 00800-03FFF (Disabled)
#pragma config WRT1 = OFF       // Table Write Protect 04000-07FFF (Disabled)
#pragma config WRT2 = OFF       // Table Write Protect 08000-0BFFF (Disabled)
#pragma config WRT3 = OFF       // Table Write Protect 0C000-0FFFF (Disabled)

// CONFIG6H
#pragma config WRTC = OFF       // Config. Write Protect (Disabled)
#pragma config WRTB = OFF       // Table Write Protect Boot (Disabled)
#pragma config WRTD = OFF       // Data EE Write Protect (Disabled)

// CONFIG7L
#pragma config EBRT0 = OFF      // Table Read Protect 00800-03FFF (Disabled)
#pragma config EBRT1 = OFF      // Table Read Protect 04000-07FFF (Disabled)
#pragma config EBRT2 = OFF      // Table Read Protect 08000-0BFFF (Disabled)
#pragma config EBRT3 = OFF      // Table Read Protect 0C000-0FFFF (Disabled)

// CONFIG7H
#pragma config EBRTB = OFF      // Table Read Protect Boot (Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <xc.h>
#include "TM1637PIC.h"

#define _XTAL_FREQ 64000000     // Define clock frequency used by xc8 __delay(time) functions

#define LED1ON 0x08         // Mask to set port RG3 output
#define LED2ON 0x10         // Sets RG4
#define IPENABLE 0          // 1 = enable two level interrupt priority (high/low)
#define BLANKON 1

//Timer1 definitions:
#define T1PRESCALE 0x03                // 2 bits control, 11 = 0x03 = 1:8
#define TIMER1ON 0x01                  // Used to set bit0 T1CON = Timer1 ON

// Timer1 setup values for 50ms interrupt using a preload:
#define TIMER1LOWBYTE 0x60             // Cycle time @ 64MHz = 0.0625us. Interrupt interval = 25ms = 400000 cycles
#define TIMER1HIGHBYTE 0x3C            // Prescale x8 = 50000 counts. Preload 65536-50000 = 15536 = 0x3C60

// Task management definitions, define stages of non-blocking task here:
#define NOTASK 0
#define TASKSTART 1
#define TASKSTAGE 2
#define TASKEND 3

//TM1637 definitions and variables, including port/pin setup:
#define DISPLAY4DIG1TO4 1                 // TM1637 display types, this is standard 4 digit, 0..3 from left
#define DISPLAY6DIG1TO6 6                 // 6 digit, digits 0..5 from left
#define DISPLAY6DIG321654 7               // 6 digit, Chinese made board with 2..0 5..3 pattern
uint8_t *portLatch = (uint8_t*)&LATG;     // Set up a pointer for writes to the TM1637 port latch, cast avoids LAT volatile warning
uint8_t *portPins = (uint8_t*)&PORTG;     // For port reads need to use the actual port not latch address
uint8_t *portTris = (uint8_t*)&TRISG;     // Also a pointer for TRIS address
uint8_t dioBit = 1;                       // This is the bit *SHIFT* (not mask) to set or clear data TRIS or PORT bits
uint8_t clkBit = 2;                       // Clock TRIS bit shift. dio is on b1 = RG1,clk on b2 = RG2 etc
const uint8_t dispType = DISPLAY6DIG1TO6;
const uint8_t decimalPos = 99;            // Flag for decimal point (digits counted from left 0..n),if > MaxDigits dp off
const uint8_t round = 0;                  // Number of digits to round, from right, 0 = no rounding, zeros rounded digits
const uint8_t ldgZeroB = 0;               // If set true blanks leading zeros
const uint8_t rightShift = 0;             // Right shifts displayed digits, discarding values on right, use after rounding
const uint8_t brightness = 2;             // Brightness, max 7, 0 is off

// General global variables:
volatile uint8_t timer1Flag = 0;               // Flag is set by Timer 1 ISR every 25ms
uint8_t counts25ms = 0;                        // Counts intervals for ADC task in 25ms increments
uint8_t keyReadCounter = 0;                    // Counts interval for key reads
uint8_t keyPressed = 0;                        // Stores the value of a valid key rea
uint8_t taskStatus = 0;                        // Stage of task, 0 = not started
uint8_t LEDcounter = 0;                        // Used to time non-blocking LED flash in 50ms increments
uint8_t LEDonTime = 0;                         // If true LED flash routine is called, flashes N x 25ms 

// Function prototypes:
void __interrupt(high_priority) ISRhi(void);   // Note XC8 interrupt setup syntax using __interrupt(optional arguments) + myisr()
void initialise18F66K90(void);                 // If using 2 level interrupts with 18F PIC also need a low_priority ISR
void LEDflash(void);



void main(void)
{
  uint32_t outputInteger = 999999;                      // Note int 65K limit - if larger than 4 digit display must use uint32_t
  _delay(100);
  initialise18F66K90();                             // LED and Timer1 with interrupts set up
  tm1637initialise(portTris,portPins,portLatch,dioBit,clkBit,dispType,brightness);
  counts25ms = 0;                                  // Start with timing counts at zero, both ADC read and timer1 flags
  timer1Flag = 0; 
  tm1637output(outputInteger, decimalPos , round, ldgZeroB, rightShift);
  T1CON |= TIMER1ON;
  while(1)
    {
      if (timer1Flag)
        {
           counts25ms ++;                    // Update task interval and LED timing flags
           keyReadCounter ++;
           LEDcounter ++;                           
           timer1Flag = 0;                   // Clear the 25ms timing flag
        }
      
      if (keyReadCounter >= 8 && keyPressed < 1)     // Read keys every 200 ms, timeout once valid press
      {
          keyPressed = keyscan();
          keyReadCounter = 0;
      }
      
      if (counts25ms >= 40)                  // Start task every 25ms x 40 = 1 sec
        { 
           counts25ms = 0;
           taskStatus = TASKSTART;           // Start task every second
           LEDcounter = 0;                   // Zero the LED time counter, note counts 50ms increments
           LEDonTime = 1;                    // Sets up a 500ms LED flash
           outputInteger ++;
           if(outputInteger > 999999)
               outputInteger = 0;
           if (keyPressed < 1)
               tm1637output(outputInteger, decimalPos , round, ldgZeroB, rightShift);   
           else
             {
               tm1637output(keyPressed, decimalPos , round, BLANKON, rightShift);  // Display the key
               keyPressed = 0;
               keyReadCounter = 0;                         // Zeroing flags will re enable key reads
             }
        }
      
      switch (taskStatus)                   // The task is managed by taskStatus control variable
      {                                     // Example code section shows framework for non-blocking task code
          case NOTASK:
            break;
                  
          case TASKSTART:
            {
              taskStatus = NOTASK;       // No task action this demo, Insert task code and update taskStatus here
              break;
            }
          
          case TASKSTAGE:
            {
              taskStatus = NOTASK;       // Insert task code and update taskStatus here, stages as needed
              break;
            } 
          
          case TASKEND:
            {
              taskStatus = NOTASK;       // End of task
              break;
            }
      }
             
      if (LEDonTime)                     // Call the LED flash function if a count is set
          LEDflash();   
    }                       //while(1)
}        

//***************************************************************************************
// Interrupt service routine:
//***************************************************************************************

void ISRhi(void)
{ 
    if (PIR1 & 0x01)                  // Check Timer1 interrupt flag bit 0 is set
    {
        PIR1 &= 0xFE;                 // Clear interrupt flag bit 0
                                      // Reset Timer1 preload for 100ms overflow/interrupt(nb running timer)
        TMR1H = TIMER1HIGHBYTE;       // Note some timing inaccuracy due to interrupt latency + reload time
        TMR1L = TIMER1LOWBYTE;
        timer1Flag = 1;               
        
    }
}

//*******************************************************************************************
//Functions: 
//*******************************************************************************************

void LEDflash(void)
{
    if (LEDcounter <= LEDonTime)
    {
        *portLatch |= LED1ON;                      // LED1 on
    }
    else
    {
        *portLatch &= ~LED1ON;                     // LED1 off
         LEDonTime = 0;                            // Stop the flash
    }
}

void initialise18F66K90(void)      // Beware PIC variations of the T1CON bits! This PIC has RD16 on b1, differs from others
{ 
    ANCON2 &= ~(0x0E);             // Clear b1..3, RG3->1 set as digital inputs, disable AN17..19, note default is ANALOG
    PORTG = 0;                     // Clear the outputs first
    TRISG = 0;                     // TM1637 TRIS (default state): All pins set as digital outputs
                                   // though TM1637 initialisation will later modify the settings.
    RCON |= IPENABLE;              // Enables two level interrupt priority if b7 set
    
    // TIMER1 setup with interrupts, timer is not yet running after initialisation:
    T1CON = 0x02;                  // T1CON clear apart from b1, RD16, 16 bit writes enabled, b3,6,7 clear = use int clk F/4 
    T1CON |= (T1PRESCALE<<4);      // Bits 4-5 set prescale, 01 = 1:2
    T1CON |= 0x04;                 // Bit 2 set enables disables external clock input 
    TMR1L = TIMER1LOWBYTE;         // Set Timer1 preload for 1ms overflow/interrupt
    TMR1H = TIMER1HIGHBYTE; 
    PIE1 = 0x01;                   // Timer 1 interrupt enable b0 set, other interrupts disabled
    PIR1 &= 0xFE;                  // Clear Timer1 interrupt flag bit 0
    INTCON |= 0xC0;                // Enable interrupts, general - bit 7 plus peripheral - bit 6 
}
