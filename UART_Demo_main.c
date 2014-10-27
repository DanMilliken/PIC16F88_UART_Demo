/******************************************************************************
 *
 * PIC16F88_UART_Demo
 *
 * Author: Dan Milliken
 * Date: 2014-10-24
 * Project: PIC16F88_UART_Demo
 * Description: Demonstrates using the PIC16F88 Microcontroller with a MAX232 IC
 * to produce RS232 output to a PC.
 *
 * License: Licensed under the Creative Commons Attribution-ShareAlike 4.0
 * International License (http://creativecommons.org/licenses/by-sa/4.0/)
 *
*******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <stdio.h>         /* C standard IO */

#include <PIC16F88.h>

#include <limits.h>

#define _XTAL_FREQ  20000000
#define _TMR0_START 100
#define _TMR0_PS    32
#define _TMR0_COUNT 1

#pragma config LVP = OFF        // disable low voltage programming
#pragma config FCMEN = OFF      // disable fail safe clock monitor
#pragma config IESO = OFF       // disable internal/external oscillator switchover
#pragma config BOREN = OFF      // disable brown out reset
#pragma config PWRTE = ON       // enable power up timer
#pragma config WDTE = OFF       // disable watchdog timer
#pragma config FOSC = HS        // external crystal

// function prototypes
void UART_write(unsigned char c);
void UART_init(void);
void timer0_init(void);
void UART_put_line(const char * str);
void interrupt ISR(void);
void CheckForButtonPress(void);

// globals
unsigned long long system_time = 0;   // ms - SYSTEM RUNNING TIME LIMIT: 49 days
unsigned char tmr0_count = 0;
unsigned char pressCount = 0;
unsigned char buttonState = 0; // 0 - up   1 - down

int main(void) {

    timer0_init(); // initialize the system time
    UART_init();  // initialize the UART module

    TRISAbits.TRISA0 = 1;  // Configure RA0 as input (for pushbutton)
    TRISAbits.TRISA1 = 0;  // Configure RA1 as output (for LED)
    ANSELbits.ANS0 = 0;    // Set RA0 to digital
    ANSELbits.ANS1 = 0;    // Set RA1 to digital

    INTCONbits.PEIE = 1; // enable peripheral interrupts
    INTCONbits.GIE = 1;  // enable interrupts

    while(1) {
        CheckForButtonPress();
    }
}

void CheckForButtonPress()
{
//    unsigned long long CURRENT_TIME = system_time;

    if (1 == RA0 && 0 == buttonState) // up to down
    {
        pressCount++;
        printf("%u: UP->DOWN\n", system_time);
        buttonState = RA0;
        RA1 = 1; // Turn on LED
    }
    else if (0 == RA0 && 1 == buttonState)
    {
        printf("%u: DOWN->UP\n", system_time);
        buttonState = RA0;
        RA1 = 0; // Turn off LED
    }

}

void timer0_init(void)
{
    OPTION_REGbits.PSA = 0;  // Prescalar is assigned to Timer0
    OPTION_REGbits.T0CS = 0; // Internal instruction clock cycle
    OPTION_REGbits.PS2 = 1;  // Prescalar = 32
    OPTION_REGbits.PS1 = 0;  // TODO: set based on define above
    OPTION_REGbits.PS0 = 0;
    TMR0 = _TMR0_START;
    INTCONbits.TMR0IE = 1;   // Timer0 interrupt enable
}

void UART_init(void)
{
    TXSTAbits.BRGH = 1; // high baud rate
    TXSTAbits.SYNC = 0; // asynchronous mode
    TXSTAbits.TX9  = 0; // 8-bit transmission
    RCSTAbits.CREN = 1; // continuous receive enable

    SPBRG = 129;        // 9600 baud @ 20MHz

    PIE1bits.RCIE  = 1;
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 1; // enable transmitter

    return;
}

void UART_write(unsigned char c) {
    while (!TXSTAbits.TRMT);
    TXREG = c;

    return;
}

void UART_put_line(const char * str)
{
    while (*str != '\0')
    {
        UART_write(*str);
        str++;
    }
    UART_write('\r');   // Write CR
    UART_write('\n');   // Write LF

    return;
}

// Override putch called by printf
void putch(unsigned char byte)
{
    UART_write(byte);
    if ('\n' == byte)
        UART_write('\r');
    return;
}

void interrupt ISR(void) {

    // AUSART Receive Interrupt Flag bit
    if (1 == PIR1bits.RCIF)
        UART_write(RCREG);

    // Timer 0 overflow interrupt
    if (1 == INTCONbits.TMR0IF)
    {
        INTCONbits.TMR0IE = 0;   // Timer0 interrupt disable
        tmr0_count++;

        if (_TMR0_COUNT == tmr0_count)    // TODO: Look at assembly and use shift register to optimize
        {
            system_time++;
            tmr0_count = 0;
        }
        
        INTCONbits.TMR0IF = 0;
        TMR0 = _TMR0_START;
        INTCONbits.TMR0IE = 1;   // Timer0 interrupt enable
    }

    return;
}

