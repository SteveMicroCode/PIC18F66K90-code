// *****************************************************************************
// PIC 18F66K90 Hello World
// Test code for MPLAB C8 that flashes two on board LEDs
//
// Hardware:
// PIC 18F66K90 with 5V power,16 MHz crystal, can use x4 PLL, ENABLED, *cannot* use XTAL>25MHz this chip
// ENVREG pin connected to Vdd allows 5V supply voltage
// Max clock speed is set up, 64 MHz, 16 MIPS
// Standard PicKit3 ICSP connections, MCLR to +5V via 10K resistor
// RG3 pin 6 LED1 (red) + 2k resistor      nb. PORTG on this PIC restricted to 2mA output,use high brightness LED
// RG4 pin 8 LED2 (green) + 2k resistor
//
// Demo code only no warranty of any kind implied
// Author: Steve Williams 14/10/2023
// *****************************************************************************


// PIC18F66K90 Configuration Bit Settings:

// CONFIG1L
#pragma config RETEN = ON       // VREG Sleep Enable bit (Enabled)
#pragma config INTOSCSEL = HIGH // LF-INTOSC Low-power Enable bit (LF-INTOSC in High-power mode during Sleep)
#pragma config SOSCSEL = HIGH   // SOSC Power Selection and mode Configuration bits (High Power SOSC circuit selected)
#pragma config XINST = OFF      // Extended Instruction Set (Enabled)

// CONFIG1H
#pragma config FOSC = HS1       // Oscillator (HS oscillator (Medium power, 4 MHz - 16 MHz))  // Note 16MHzXTAL
#pragma config PLLCFG = ON      // PLL x4 Enable bit (Enabled, 64 MHz clock)
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

#define _XTAL_FREQ 64000000      // Define clock frequency used by xc8 __delay(time) functions

#define LED1ON 0x08         // Mask to set port RG3 output
#define LED1OFF 0xF7        // Mask to clear port RG4 output
#define LED2ON 0x10         // Sets RG4
#define LED2OFF 0xEF        // Clears RG4

void main(void)
{   
    PORTG = 0;               // Clear the outputs first
    TRISG = 0;               // Port G set up so all bits are outputs and are low (RG0 .. 4)
    while(1)
    {
      __delay_ms(1000);            // Output alternating short(0.1 sec) then long(1sec) LED flashes
      PORTG |= LED1ON;             // LED1 to indicate PIC is alive ... "Hello World"
      __delay_ms(100);             // This delay is 100 msec, uses inline compiler function
      PORTG &= LED1OFF;
      __delay_ms(500);             // 0.5 sec
      PORTG |= LED1ON;
      __delay_ms(1000);            // 1 sec
      PORTG &= LED1OFF;
      
      //Now LED2:
      __delay_ms(1000);            // Output alternating short(0.1 sec) then long(1sec) LED flashes
      PORTG |= LED2ON;
      __delay_ms(100);             // Delay 0.1 sec
      PORTG &= LED2OFF;
      __delay_ms(500);             // 0.5 sec
      PORTG |= LED2ON;
      __delay_ms(1000);            // 1 sec
      PORTG &= LED2OFF;
    }
}






