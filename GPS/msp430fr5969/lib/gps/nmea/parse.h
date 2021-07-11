/*
 * parse.h
 *
 *  Created on: 27 de jan de 2021
 *      Author: Jose Gustavo
 *
 *  Based on: NMEA library
 *      URL: http://nmea.sourceforge.net
 *      Author: Tim (xtimor@gmail.com)
 *      Licence: http://www.gnu.org/licenses/lgpl.html
 *      $Id: parse.h 4 2007-08-27 13:11:03Z xtimor $
 */

#ifndef LIB_GPS_NMEA_PARSE_H_
#define LIB_GPS_NMEA_PARSE_H_

#include <lib/gps/nmea/sentence.h>

unsigned short int nmea_parse_GPRMC(char *buff, nmeaGPRMC *pack);
unsigned short int nmea_pack_type(char *buff, unsigned short int buff_sz);
char* calc_checksum(char *buff);

#endif /* LIB_GPS_NMEA_PARSE_H_ */
