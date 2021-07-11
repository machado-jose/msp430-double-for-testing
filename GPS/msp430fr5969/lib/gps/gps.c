/*
 * gps.c
 *
 *  Created on: 27 de jan de 2021
 *  Author: Jose Gustavo
 */

#include <lib/gps/nmea/parse.h>
#include <lib/gps/nmea/sentence.h>
#include <msp430.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>


void send_command(char *command){
	int checksum = 0, size_command = (int)strlen(command), size_buff;
	unsigned int i;

	for(i = size_command; i > 0; --i){
		checksum ^= command[size_command - i];
	}

	char *buff = malloc(size_command + sizeof(checksum) + 5);
	sprintf(buff, "%s%s%s%d%s", "$", command, "*", checksum, "\r\n");

	size_buff = (int)(strlen(buff));

    for(i = size_buff; i > 0; --i){
        UCA1TXBUF = buff[size_buff - i];
        __delay_cycles(1009);
    }

	free(buff);

}

char* get_checksum(char *buff){
    return calc_checksum(buff);
}

unsigned short int verify_checksum(char* buff, char *chsum_calculed){
    char *bkp_aux, *chsum;
    unsigned short int res;

    bkp_aux = malloc(strlen(buff) + 1);
    strcpy(bkp_aux, buff);

    /** Verify sentence checksum **/
    chsum = strrchr(bkp_aux, '*');

    if(chsum == NULL){
        free(chsum);
        free(bkp_aux);
        return 0;
    }

    memmove(&chsum[0], &chsum[1], strlen(chsum));

    if(strncmp(chsum, chsum_calculed, strlen(chsum_calculed)) != 0){
        res = 0;
    }else{
        res = 1;
    }

    free(chsum);
    free(bkp_aux);
    return res;
}

void send(char *command){
    int i, size_command = strlen(command);
    for(i = size_command; i > 0; --i){
        UCA1TXBUF = command[size_command - i];
        __delay_cycles(1009);
    }
}

void send_received_command(char *received_command_buff){
    int size_command, i;
    memmove(&received_command_buff[0], &received_command_buff[1], strlen(received_command_buff));
    size_command = strlen(received_command_buff);
    for(i = size_command; i > 0; --i){
        if(received_command_buff[size_command - i] == '*'){
            break;
        }
        UCA0TXBUF = received_command_buff[size_command - i];
        __delay_cycles(1009);
    }
}

void received_command(char *command){
    int size_command_buff = strlen(command);
    unsigned int i;

    for(i = size_command_buff; i > 0; --i){
        UCA0TXBUF = command[size_command_buff - i];
        __delay_cycles(5000);
    }

}

void update_information(char *command, nmeaGPRMC *pack){
    nmea_parse_GPRMC(command, pack);
}

void get_position(nmeaGPRMC *pack){
    char lat[12], lon[12];
    unsigned short int info_sz, i;

    if(pack->lat != NULL && pack->lon != NULL){
        sprintf(lat, "%f%s", pack->lat, "\r\n");
        sprintf(lon, "%f%s", pack->lon, "\r\n");

        info_sz = strlen(lat);
        for(i = info_sz; i > 0; --i){
            UCA0TXBUF = lat[info_sz - i];
            __delay_cycles(5000);
        }

    }
}
