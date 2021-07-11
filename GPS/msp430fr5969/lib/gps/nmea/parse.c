/*
 * parse.c
 *
 *  Created on: 27 de jan de 2021
 *      Author: Jose Gustavo
 *
 *  Based on: NMEA library
 *      URL: http://nmea.sourceforge.net
 *      Author: Tim (xtimor@gmail.com)
 *      Licence: http://www.gnu.org/licenses/lgpl.html
 *      $Id: parse.c 17 2008-03-11 11:56:11Z xtimor $
 */

#include <lib/gps/nmea/parse.h>
#include <lib/gps/nmea/tok.h>
#include <string.h>
#include <stdio.h>


#define GPRMC (1)

/**
 * \brief Define packet type by header.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @return The defined packet type
*/

unsigned short int nmea_pack_type(char *buff, unsigned short int buff_sz)
{
    /* Save the command types */
    static const char *pheads[] = {
        "$GPRMC"
    };

    if(buff_sz < 6)
        return 0;
    else if(0 == memcmp(buff, pheads[0], 5))
        return GPRMC;

    return 0;
}

/**
 * \brief Parse RMC packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */

unsigned short int nmea_parse_GPRMC(char *buff, nmeaGPRMC *pack)
{
    unsigned short int buff_sz;
    buff_sz = strlen(buff);

    if(nmea_pack_type(buff, buff_sz) == GPRMC){

        memset(pack, 0, sizeof(nmeaGPRMC));

        if(nmea_scanf(buff, pack) == 0)
        {
            memset(pack, 0, sizeof(nmeaGPRMC));
            return 0;
        }

        return 1;
    }

    return 0;
}

char* calc_checksum(char *buff){
    return checksum(buff);
}
