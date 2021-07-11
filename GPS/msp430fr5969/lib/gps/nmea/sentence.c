/*
 * sentence.c
 *
 *  Created on: 27 de jan de 2021
 *      Author: Jose Gustavo
 *
 *  Based on: NMEA library
 *      URL: http://nmea.sourceforge.net
 *      Author: Tim (xtimor@gmail.com)
 *      Licence: http://www.gnu.org/licenses/lgpl.html
 *      $Id: sentence.c 17 2008-03-11 11:56:11Z xtimor $
 */

#include <lib/gps/nmea/sentence.h>
#include <lib/gps/nmea/time.h>
#include <string.h>


void nmea_zero_GPRMC(nmeaGPRMC *pack)
{
    memset(pack, 0, sizeof(nmeaGPRMC));
    nmea_time_init(&pack->utc);
}

