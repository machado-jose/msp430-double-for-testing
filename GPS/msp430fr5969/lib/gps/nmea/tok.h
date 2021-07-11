/*
 * tok.h
 *
 *  Created on: 27 de jan de 2021
 *      Author: Jose Gustavo
 *
 *  Based on: NMEA library
 *      URL: http://nmea.sourceforge.net
 *      Author: Tim (xtimor@gmail.com)
 *      Licence: http://www.gnu.org/licenses/lgpl.html
 *      $Id: tok.h 4 2007-08-27 13:11:03Z xtimor $
 */

#ifndef LIB_GPS_NMEA_TOK_H_
#define LIB_GPS_NMEA_TOK_H_

#include <lib/gps/nmea/sentence.h>

unsigned short int nmea_scanf(char *buff, nmeaGPRMC *pack);
unsigned short int nmea_calc_crc(char *buff, unsigned short int buff_sz);
double nmea_atof(char *str, int str_sz);
char* checksum(char *buff);
//char* shift(char *buff);

#endif /* LIB_GPS_NMEA_TOK_H_ */
