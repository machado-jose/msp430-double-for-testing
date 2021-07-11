/*
 * time.c
 *
 *  Created on: 27 de jan de 2021
 *      Author: Jose Gustavo
 *
 *  Based on: NMEA library
 *      URL: http://nmea.sourceforge.net
 *      Author: Tim (xtimor@gmail.com)
 *      Licence: http://www.gnu.org/licenses/lgpl.html
 *      $Id: time.c 4 2007-08-27 13:11:03Z xtimor $
 */

#include <lib/gps/nmea/time.h>

void nmea_time_init(nmeaTIME *stm)
{
    stm->year = 0;
    stm->mon = 0;
    stm->day = 0;
    stm->hour = 0;
    stm->min = 0;
    stm->sec = 0;
    stm->hsec = 0;
}
