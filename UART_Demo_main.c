/******************************************************************************
 *
 * PIC16F88_UART_Demo
 *
 * Author: Dan Milliken
 * Date: 2014-12-15
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

#include <stdio.h>         /* C standard IO */
#include <string.h>

// Program parameters
#define INPUT_LENGTH 64        // Size of user input string.

#define _XTAL_FREQ  4000000

#pragma config LVP = OFF        // disable low voltage programming
#pragma config FCMEN = OFF      // disable fail safe clock monitor
#pragma config IESO = OFF       // disable internal/external oscillator switchover
#pragma config BOREN = OFF      // disable brown out reset
#pragma config PWRTE = ON       // enable power up timer
#pragma config WDTE = OFF       // disable watchdog timer
#pragma config FOSC = INTOSCIO  // Internal oscillator

// function prototypes
void interrupt ISR(void);
void UART_init(void);
void UART_write(unsigned char c);

int main(void)
{
    OSCCONbits.IRCF = 0b110; // Set internal RC oscillator to 4 MHz
    while(!OSCCONbits.IOFS); // Wait for frequency to stabalize

    UART_init();  // initialize the UART module

    INTCONbits.GIE = 1;  // enable interrupts

    printf("*** UART Demo System startup ***\n");
    
    while(1) {
        char input_string[INPUT_LENGTH+2], reverse_string[INPUT_LENGTH];
        int string_length = 0;
        char * ptr_input_string;
        memset(input_string,0,INPUT_LENGTH);
        memset(reverse_string,0,INPUT_LENGTH);
        input_string[0] = INPUT_LENGTH;
        printf("Enter a string:\n");
        ptr_input_string = cgets(input_string);
        string_length = input_string[1];
        printf("The entered string:\n");
        printf("%s\n",ptr_input_string);

        for (int c = string_length - 1, d = 0; c >= 2; c--, d++)
            reverse_string[d] = input_string[c+2];

        printf("Reversed:\n");
        printf("%s\n",reverse_string);
    }
}

void UART_init(void)
{
    TXSTAbits.BRGH = 1; // high baud rate
    TXSTAbits.SYNC = 0; // asynchronous mode
    TXSTAbits.TX9  = 0; // 8-bit transmission
    RCSTAbits.CREN = 1; // continuous receive enable

    SPBRG = 25;         // 9600 baud @ 4MHz with BRGH = 1

    PIE1bits.RCIE  = 1; // Enable receive interrupt
    RCSTAbits.SPEN = 1; // Enable the port
    TXSTAbits.TXEN = 1; // enable transmitter

    return;
}

void UART_write(unsigned char c) {
    while (!TXSTAbits.TRMT);
    TXREG = c;

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

    return;
}

