/*
 * main.c
 *
 *  Created on: 27 de jan de 2021
 *  Author: Jose Gustavo
 */

#include <msp430.h>
#include <string.h>

#include "lib/gps/gps.h"
#include "lib/gps/nmea/sentence.h"

void SetupUartTerminal(void);
void SetupUartGPS(void);
void SetupRTC(void);
void ConfigureLFXT(void);
void SetupLedPins(void);
void ToggleLed(void);

unsigned short int sentence_ctl = 0;
unsigned short int send_command_each_second = 0;

/* Commands to Send */
char *rmc_message = "$GPRMC,141623.523,A,2143.963,S,04111.493,W,,,301019,000.0,W*7B";
char *update_rate_1s = "$PMTK220,1000";

/* Received Command Buffer */
char received_command_buff[80];
unsigned int rx_byte_cnt = 0;

nmeaGPRMC pack;

int main(void)
{
    nmea_zero_GPRMC(&pack);

    WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog
    PM5CTL0 &= ~LOCKLPM5;

    SetupLedPins();
    SetupUartTerminal();
    SetupUartGPS();
    SetupRTC();
    ConfigureLFXT();

    update_information(rmc_message, &pack);

    __enable_interrupt();

    while(1){}

}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    char checksum[6];
    switch(UCA0RXBUF){
        case('y'):
            ToggleLed();
            memset(checksum, 0, sizeof(checksum));
            strcpy(checksum, get_checksum(received_command_buff));
            if(verify_checksum(received_command_buff, checksum) == 1){
                if(strstr(received_command_buff, update_rate_1s) != NULL){
                    send(rmc_message);
                }else{
                    send_received_command(received_command_buff);
                }
            }
            memset(received_command_buff, 0, sizeof(received_command_buff));
            rx_byte_cnt = 0;
            memset(checksum, 0, sizeof(checksum));
            ToggleLed();
            break;
        case('a'):
            ToggleLed();
            memset(checksum, 0, sizeof(checksum));
            strcpy(checksum, get_checksum(received_command_buff));
            if(verify_checksum(received_command_buff, checksum) == 1){
                if(strstr(received_command_buff, update_rate_1s) != NULL){
                    send_command_each_second = 1;
                }
            }
            memset(received_command_buff, 0, sizeof(received_command_buff));
            rx_byte_cnt = 0;
            memset(checksum, 0, sizeof(checksum));
            ToggleLed();
            break;
        case('x'):
            send_command_each_second = 0;
        default:
            break;
    }
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
    received_command_buff[rx_byte_cnt] = UCA1RXBUF;
    rx_byte_cnt++;
}

void SetupLedPins(void){
    /* Set Led */
    P1DIR |= BIT0;                            // Green Led
    P1OUT |= BIT0;                            // Led on
    P4DIR |= BIT6;                            // Red Led
    P4OUT &= ~BIT6;                           // Led off
}

void ToggleLed(void){
    P1OUT ^= BIT0;                            // Toggle Green Led
    __delay_cycles(2000);
    P4OUT ^= BIT6;                            // Toggle Red Led
    __delay_cycles(2000);
}

void SetupUartTerminal(void){
    /* Setup UART A0 - 115200 baud */
    UCA0CTLW0 |= UCSWRST;                     // SW Reset

    UCA0CTLW0 |= UCSSEL__SMCLK;               // BRCLK = SMCLK (1MHz)
    UCA0BRW = 8;                              // Divisor = 8
    UCA0MCTLW = 0xD600;                       // Modulation = 0xD600; UCBRS = 0xD6; UCBRF = 0; UCOS16 = 0

    P2SEL1 |= BIT0;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT0;                          // To transmit test result from computer terminal (/dev/ttyACM*)

    P2SEL1 |= BIT1;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT1;                          // To receive test command from terminal

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void SetupUartGPS(void){
    /* Setup UART A1 - 9600 baud */
    UCA1CTLW0 |= UCSWRST;                     // SW Reset

    UCA1CTLW0 |= UCSSEL__SMCLK;               // BRCLK = SMCLK (1MHz)
    UCA1BRW = 6;                              // Divisor = 6
    UCA1MCTLW = 0x2081;                       // Modulation = 0x2081; UCBRS = 0x20; UCBRF = 8; UCOS16 = 1

    P2SEL1 |= BIT5;                           // USCI_A1 UART operation
    P2SEL0 &= ~BIT5;                          // To transmit GPS command

    P2SEL1 |= BIT6;                           // USCI_A1 UART operation
    P2SEL0 &= ~BIT6;                          // To received GPS command

    UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

    UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
}

void SetupRTC(void){
    // Configure RTC_C
    RTCCTL01 = RTCTEVIE | RTCRDYIE | RTCBCD | RTCHOLD;
                                            // RTC enable, BCD mode, RTC hold
                                            // enable RTC read ready interrupt
                                            // enable RTC time event interrupt

    RTCYEAR = 0x20;                         // Year
    RTCMON = 0x00;                          // Month
    RTCDAY = 0x01;                          // Day
    RTCDOW = 0x00;                          // Day of week
    RTCHOUR = 0x00;                         // Hour
    RTCMIN = 0x00;                          // Minute
    RTCSEC = 0x00;                          // Seconds
    RTCCTL01 &= ~(RTCHOLD);                 // Start RTC
}

void ConfigureLFXT(void){
    PJSEL0 = BIT4 | BIT5;                   // Initialize LFXT pins
    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
    do
    {
      CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
      SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);              // Test oscillator fault flag
    CSCTL0_H = 0;                           // Lock CS registers
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
    switch(__even_in_range(RTCIV, RTCIV_RT1PSIFG))
    {
        case RTCIV_NONE:      break;        // No interrupts
        case RTCIV_RTCOFIFG:  break;        // RTCOFIFG
        case RTCIV_RTCRDYIFG:               // RTCRDYIFG
            if(send_command_each_second == 1){
                P1OUT ^= BIT0;
                send(rmc_message);
            }
            break;
        case RTCIV_RTCTEVIFG: break;        // RTCEVIFG
        case RTCIV_RTCAIFG:   break;        // RTCAIFG
        case RTCIV_RT0PSIFG:  break;        // RT0PSIFG
        case RTCIV_RT1PSIFG:  break;        // RT1PSIFG
        default: break;
    }
}
