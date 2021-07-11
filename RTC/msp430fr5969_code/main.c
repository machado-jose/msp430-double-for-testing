#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SLAVE_ADDRESS 0x68
#define MAX_BUFFER_SIZE 15

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t dow;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} TimeBuff;

uint8_t ReceiveBuffer[MAX_BUFFER_SIZE] = {0};
uint8_t ReceiveIndex = 0;
uint8_t TransmitIndex = 14;
uint8_t rtc_activated = 0;

TimeBuff time_buff;

char input[100];
unsigned int input_index = 0;
char terminal_buff[25];
unsigned int terminal_index = 0;

void Setup_I2C_Slave(void);
void Setup_UART(void);
void SetupTimeBuff(void);
void Configure_GPIO_I2C(void);
void Configure_Leds(void);
void ConfigureLFXT(void);
void ConfigureRTC(void);
void TransmitDataUART(uint8_t data);
void TransmitMasterData(uint8_t *buff);
void TransmitTimeBuffData(void);
void sendSecond(void);
void sendMinute(void);
void sendHour(void);
void sendDayOfWeek(void);
void sendDay(void);
void sendMonth(void);
void sendYear(void);
void sendTime(void);
void SetRTC(void);
void SetRTCRegisters(void);
void SetTimeBuff(void);
void ResetReceiveBuffer(void);
void ActiveRTC(void);
void getTimeInteger(char *data);
uint8_t SendNextTimeFromBuff(void);
uint8_t SendNextTimeFromRTC(void);
uint8_t HexToDec(uint8_t data);
uint8_t DecToHex(uint8_t data);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    Configure_GPIO_I2C();
    Configure_Leds();
    SetupTimeBuff();
    PM5CTL0 &= ~LOCKLPM5;
    Setup_I2C_Slave();
    Setup_UART();
    ConfigureLFXT();
    ConfigureRTC();

    __bis_SR_register(LPM0_bits | GIE);       // Enter LPM0 w/ interrupts
    __no_operation();
}

void SetupTimeBuff(void){
    time_buff.second = 45;
    time_buff.minute = 33;
    time_buff.hour = 21;
    time_buff.dow = 0;
    time_buff.day = 27;
    time_buff.month = 6;
    time_buff.year = 21;
}

void ResetReceiveBuffer(void){
    unsigned int i = 0;
    ReceiveIndex = 0;
    for(i = 0; i < MAX_BUFFER_SIZE; i++){
        ReceiveBuffer[i] = 0;
    }
}

void ConfigureRTC(void){
    // Configure RTC_C
    RTCCTL01 = RTCTEVIE | RTCRDYIE | RTCBCD | RTCHOLD;
                                            // RTC enable, BCD mode, RTC hold
                                            // enable RTC read ready interrupt
                                            // enable RTC time event interrupt

    SetRTCRegisters();
    RTCCTL01 &= ~(RTCHOLD);                 // Start RTC
}

void SetRTC(void){
    SetTimeBuff();
    SetRTCRegisters();
    ResetReceiveBuffer();
}

void SetRTCRegisters(void){
    RTCYEAR = DecToHex(time_buff.year);               // Year
    RTCMON = DecToHex(time_buff.month);               // Month
    RTCDAY = DecToHex(time_buff.day);                 // Day
    RTCDOW = DecToHex(time_buff.dow);                 // Day of week
    RTCHOUR = DecToHex(time_buff.hour);               // Hour
    RTCMIN = DecToHex(time_buff.minute);              // Minute
    RTCSEC = DecToHex(time_buff.second);              // Seconds
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

void Setup_I2C_Slave(void){
    // Configure USCI_B0 for I2C mode
    UCB0CTLW0 = UCSWRST;                      // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCSYNC;           // I2C mode, sync mode
    UCB0I2COA0 = SLAVE_ADDRESS | UCOAEN;      // own address is 0x68 + enable
    UCB0CTLW0 &= ~UCSWRST;                    // clear reset register
    UCB0IE |= UCTXIE0 | UCRXIE0 | UCSTPIE;              // receive, stop interrupt enable
}

void Configure_GPIO_I2C(void){
    P1SEL1 |= BIT6 | BIT7;                    // I2C pins
}

void Configure_Leds(void){
    P1DIR |= BIT0;
    P4DIR |= BIT6;
    P1OUT |= BIT0;
    P4OUT |= BIT6;
}

void Setup_UART(void){
    UCA0CTLW0 |= UCSWRST;                     // SW Reset

    UCA0CTLW0 |= UCSSEL__SMCLK;               // BRCLK = SMCLK (1MHz)
    UCA0BRW = 6;                              // Divisor = 8
    UCA0MCTLW = 0x2081;                       // Modulation = 0xD600; UCBRS = 0xD6; UCBRF = 0; UCOS16 = 0

    P2SEL1 |= BIT0;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT0;                          // To transmit test result from computer terminal (/dev/ttyACM*)

    P2SEL1 |= BIT1;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT1;                          // To receive test command from terminal

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_UART_ISR(void){
    if(UCA0RXBUF == 't'){
        /* Send RTC or Buffer data to terminal */
        if(rtc_activated == 1){
            sendTime();
        }else{
            TransmitTimeBuffData();
        }
    }else if(UCA0RXBUF == 'a'){
        /* Activate RTC. Set up with values from ReceiveBuffer if populated */
        ActiveRTC();
    }else if(UCA0RXBUF == 'x'){
        rtc_activated = 0;
        ReceiveIndex = 0;
        TransmitIndex = 14;
    }else{
        if(UCA0RXBUF == 's'){
            while(terminal_buff[0] == 0x0D){
                memmove(terminal_buff, terminal_buff + 1, strlen(terminal_buff));
            }
            getTimeInteger(terminal_buff);
            SetTimeBuff();
            terminal_index = 0;
            memset(terminal_buff, 0, sizeof(terminal_buff));
        }else{
            terminal_buff[terminal_index] = UCA0RXBUF;
            terminal_index++;
        }
    }
}

void getTimeInteger(char *data){

    char *token;
    char res[7][3];
    unsigned int i = 0, j = 0;
    uint8_t bkp[7];
    const char *s;

    token = strtok(data, " ");

    while( token != NULL ) {
        strcpy(res[i], token);
        i++;
        token = strtok(NULL, " ");
    }

    for(i = 0; i < 7; i++){
        s = res[i];
        bkp[i] = atoi(s);
    }

    for(i = 0; i < 14; i++){
        if(i % 2 != 0){
            ReceiveBuffer[i] = DecToHex(bkp[j]);
            j++;
        }else{
            ReceiveBuffer[i] = 0;
        }
    }

    free(token);
}

void TransmitMasterData(uint8_t *buff){
    unsigned int i;
    for(i = 0; i < 14; i++){
        if(i % 2 != 0){
            TransmitDataUART(HexToDec(buff[i]));
        }
    }
}

void TransmitDataUART(uint8_t data){
    char *buff;
    unsigned int i;

    buff = malloc(10);
    sprintf(buff, "%d", data);

    for(i = 0; i < strlen(buff); i++){
        UCA0TXBUF = buff[i];
        __delay_cycles(5000);
    }
    UCA0TXBUF = ',';
    __delay_cycles(5000);
    free(buff);
}

void SetTimeBuff(void){
    time_buff.year = HexToDec(ReceiveBuffer[1]);
    time_buff.month = HexToDec(ReceiveBuffer[3]);
    time_buff.day = HexToDec(ReceiveBuffer[5]);
    time_buff.dow = HexToDec(ReceiveBuffer[7]);
    time_buff.hour = HexToDec(ReceiveBuffer[9]);
    time_buff.minute = HexToDec(ReceiveBuffer[11]);
    time_buff.second = HexToDec(ReceiveBuffer[13]);
}

void TransmitTimeBuffData(void){
    TransmitDataUART(time_buff.year);
    TransmitDataUART(time_buff.month);
    TransmitDataUART(time_buff.day);
    TransmitDataUART(time_buff.dow);
    TransmitDataUART(time_buff.hour);
    TransmitDataUART(time_buff.minute);
    TransmitDataUART(time_buff.second);
}

void sendSecond(void){
    uint8_t second = HexToDec(RTCSEC);
    unsigned short int i;
    char *buff = malloc(sizeof(second));
    sprintf(buff, "%d", second);
    for(i = 0; i < strlen(buff); i++){
        UCA0TXBUF = buff[i];
        __delay_cycles(5000);
    }
    UCA0TXBUF = ',';
    free(buff);
}

void sendMinute(void){
    uint8_t minute = HexToDec(RTCMIN);
    unsigned short int i;
    char *buff = malloc(sizeof(minute));
    sprintf(buff, "%d", minute);
    for(i = 0; i < strlen(buff); i++){
        UCA0TXBUF = buff[i];
        __delay_cycles(5000);
    }
    UCA0TXBUF = ',';
    free(buff);
}

void sendHour(void){
    uint8_t hour = HexToDec(RTCHOUR);
    unsigned short int i;
    char *buff = malloc(sizeof(hour));
    sprintf(buff, "%d", hour);
    for(i = 0; i < strlen(buff); i++){
        UCA0TXBUF = buff[i];
        __delay_cycles(5000);
    }
    UCA0TXBUF = ',';
    free(buff);
}

void sendDayOfWeek(void){
    uint8_t dayOfWeek = HexToDec(RTCDOW);
    unsigned short int i;
    char *buff = malloc(sizeof(dayOfWeek));
    sprintf(buff, "%d", dayOfWeek);
    for(i = 0; i < strlen(buff); i++){
        UCA0TXBUF = buff[i];
        __delay_cycles(5000);
    }
    UCA0TXBUF = ',';
    free(buff);
}

void sendDay(void){
    uint8_t day = HexToDec(RTCDAY);
    unsigned short int i;
    char *buff = malloc(sizeof(day));
    sprintf(buff, "%d", day);
    for(i = 0; i < strlen(buff); i++){
        UCA0TXBUF = buff[i];
        __delay_cycles(5000);
    }
    UCA0TXBUF = ',';
    free(buff);
}

void sendMonth(void){
    uint8_t month = HexToDec(RTCMON);
    unsigned short int i;
    char *buff = malloc(sizeof(month));
    sprintf(buff, "%d", month);
    for(i = 0; i < strlen(buff); i++){
        UCA0TXBUF = buff[i];
        __delay_cycles(5000);
    }
    UCA0TXBUF = ',';
    free(buff);
}

void sendYear(void){
    uint8_t year =  HexToDec(RTCYEAR);
    unsigned short int i;
    char *buff = malloc(sizeof(year));
    sprintf(buff, "%d", year);
    for(i = 0; i < strlen(buff); i++){
        UCA0TXBUF = buff[i];
        __delay_cycles(5000);
    }
    UCA0TXBUF = ',';
    free(buff);
}

void sendTime(void){
    sendYear();
    sendMonth();
    sendDay();
    sendDayOfWeek();
    sendHour();
    sendMinute();
    sendSecond();
}

uint8_t SendNextTimeFromRTC(void){
    switch(TransmitIndex){
        case 14:
            return RTCYEAR;
        case 12:
            return RTCMON;
        case 10:
            return RTCDAY;
        case 8:
            return RTCDOW;
        case 6:
            return RTCHOUR;
        case 4:
            return RTCMIN;
        case 2:
            return RTCSEC;
        default:
            return 0;
    }
}

uint8_t HexToDec(uint8_t data){
    return ((data / 16) * 10 + (data % 16));
}

uint8_t DecToHex(uint8_t data){
    return ((data / 10) * 16 + (data % 10));
}

void ActiveRTC(void){
    if(ReceiveIndex == 14){
        SetRTC();
    }
    rtc_activated = 1;
}

uint8_t SendNextTimeFromBuff(void){
    switch(TransmitIndex){
        case 14:
            return ReceiveBuffer[1];
        case 12:
            return ReceiveBuffer[3];
        case 10:
            return ReceiveBuffer[5];
        case 8:
            return ReceiveBuffer[7];
        case 6:
            return ReceiveBuffer[9];
        case 4:
            return ReceiveBuffer[11];
        case 2:
            return ReceiveBuffer[13];
        default:
            return 0;
    }
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
  P1OUT ^= BIT0;
  uint8_t rx_val = 0;
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG: break;         // Vector 4: NACKIFG
    case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:                 // Vector 8: STPIFG
        P1OUT ^= BIT0;
        break;
    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
    case USCI_I2C_UCRXIFG0:                 // Vector 22: RXIFG0
        rx_val = UCB0RXBUF;
        if(ReceiveIndex < 14){
            ReceiveBuffer[ReceiveIndex++] = rx_val;
        }
        break;
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
        if(rtc_activated == 1){
            UCB0TXBUF = SendNextTimeFromRTC();
        }else{
            UCB0TXBUF = SendNextTimeFromBuff();
        }
        TransmitIndex--;
        if(TransmitIndex == 0){
            TransmitIndex = 14;
        }
        break;
    case USCI_I2C_UCBCNTIFG: break;         // Vector 26: BCNTIFG
    case USCI_I2C_UCCLTOIFG: break;         // Vector 28: clock low timeout
    case USCI_I2C_UCBIT9IFG: break;         // Vector 30: 9th bit
    default: break;
  }
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
    switch(__even_in_range(RTCIV, RTCIV_RT1PSIFG))
    {
        case RTCIV_NONE:      break;        // No interrupts
        case RTCIV_RTCOFIFG:  break;        // RTCOFIFG
        case RTCIV_RTCRDYIFG:               // RTCRDYIFG
            P4OUT ^= BIT6;                  // Toggles P1.0 every second
            if(ReceiveIndex > 0 && ReceiveIndex < 14){
                ReceiveIndex = 0;
            }
            break;
        case RTCIV_RTCTEVIFG: break;        // RTCEVIFG
        case RTCIV_RTCAIFG:   break;        // RTCAIFG
        case RTCIV_RT0PSIFG:  break;        // RT0PSIFG
        case RTCIV_RT1PSIFG:  break;        // RT1PSIFG
        default: break;
    }
}
