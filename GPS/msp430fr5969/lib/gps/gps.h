/*
 * gps.h
 *
 *  Created on: 27 de jan de 2021
 *  Author: Jose Gustavo
 */

#ifndef __GPS_H__
#define __GPS_H__

#include <lib/gps/nmea/sentence.h>

void send_command(char *command);
void received_command(char *command);
void update_information(char *command, nmeaGPRMC *pack);
void get_position(nmeaGPRMC *pack);
void send(char *command);
void send_received_command(char *received_command_buff);
char* get_checksum(char *buff);
unsigned short int verify_checksum(char* buff, char *chsum_calculed);

#endif 
