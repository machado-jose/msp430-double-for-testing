#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_SIZE 10
#define MESSAGE_SIZE 30

void SetupClock(void);
void SetupUart(void);
void SetupSPI(void);
void SetupIO(void);
void ResetBuff(void);
void ResetMessage(void);
void TransmitToTerminal(void);
void FillMessage(void);
void FillBuff(void);

char message[MESSAGE_SIZE];
unsigned int message_index = 0;
unsigned int buff[BUFF_SIZE];
unsigned int buff_size = 0;
unsigned int buff_index = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    SetupClock();
    SetupIO();
    SetupUart();
    SetupSPI();

    __bis_SR_register(LPM0_bits | GIE);       // Enter LPM0, enable interrupts

    while(1){}
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_UART_ISR(void){
    if(UCA0RXBUF == 'R'){
        ResetBuff();
        ResetMessage();
    }else if(UCA0RXBUF == 'G'){
        TransmitToTerminal();
    }else if(UCA0RXBUF != 'z'){
        FillMessage();
    }else{
        FillBuff();
    }
}

#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void){
    P1OUT ^= BIT0;
    while (!(UCB0IFG & UCTXIFG)) { };           // USCI_B0 TX buffer ready?
    if(buff_index == 0){
        UCB0TXBUF = UCB0RXBUF;
    }else{
        UCB0TXBUF = buff[buff_size - buff_index];
        buff[buff_size - buff_index] = UCB0RXBUF;
        buff_index--;
    }
    P1OUT ^= BIT0;
}

void SetupIO(void){
    // Configure GPIO
    P1SEL1 |= BIT3 | BIT6 | BIT7;             // Configure CS, SOMI and MISO
    P2SEL1 |= BIT2;                           // Configure clk
    // Led
    P1DIR |= BIT0;
    P1OUT |= BIT0;
}

void SetupClock(void){
    /* Setup SMCLK to 8MHz */
    CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
    CSCTL0_H = 0;                             // Lock CS registers
}

void SetupSPI(void){
    // Configure USCI_B0 for SPI operation
    UCB0CTLW0 = UCSWRST;                      // **Put state machine in reset**
                                            // 4-pin, 8-bit SPI slave
    UCB0CTLW0 |= UCSYNC | UCCKPH | UCMSB | UCMODE_2 | UCSTEM;
                                            // Clock polarity high, MSB
    UCB0CTLW0 |= UCSSEL__UCLK;                // ACLK
    UCB0BR0 = 0x04;                           // /4
    UCB0BR1 = 0;                              //
    UCB0CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**
    UCB0IE |= UCRXIE;                         // Enable USCI_B0 RX interrupt
}

void SetupUart(void){
    /* Setup UART A0: 115200 baud */
    UCA0CTLW0 |= UCSWRST;                     // SW Reset

    UCA0CTLW0 |= UCSSEL__SMCLK;               // BRCLK = SMCLK (8MHz)
    UCA0BRW = 4;                              // Prescalar = 4
    UCA0MCTLW = 0x5551;                       // Modulation = 0x5551; UCBRS = 0x55; UCBRF = 5; UCOS16 = 1

    P2SEL1 |= BIT0;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT0;                          // To transmit test result from computer terminal (/dev/ttyACM*)

    P2SEL1 |= BIT1;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT1;                          // To receive test command from terminal

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void ResetBuff(void){
    unsigned int idx = 0;
    for(idx = 0; idx < BUFF_SIZE; idx++){
        buff[idx] = 0;
    }
    buff_size = 0;
    buff_index = 0;
}

void ResetMessage(void){
    memset(message, 0, MESSAGE_SIZE);
    message_index = 0;
}

void TransmitToTerminal(void){
    unsigned int i = 0, j = 0;
    char transmit_data[5];
    for(i = buff_size; i > 0; i--){
        sprintf(transmit_data, "%d", buff[buff_size - i]);
        for(j = 0; j < strlen(transmit_data); j++){
            UCA0TXBUF = transmit_data[j];
            __delay_cycles(5000);
        }
        UCA0TXBUF = ',';
    }
    ResetMessage();
}

void FillMessage(void){
    message[message_index] = UCA0RXBUF;
    message_index++;
}

void FillBuff(void){
    char *token, *message_bkp;
    const char *s;
    unsigned int i = 0;

    ResetBuff();

    message_bkp = message;
    token = strtok(message_bkp, " ");

    while( token != NULL ) {
        s = token;
        if(i < BUFF_SIZE){
            buff[i] = atoi(s);
            i++;
        }
        token = strtok(NULL, " ");
    }

    buff_size = i;
    buff_index = i;
    ResetMessage();
    free(token);
    free(message_bkp);
}
