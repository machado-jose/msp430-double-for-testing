/*
 * gmath.c
 *
 *  Created on: 27 de jan de 2021
 *      Author: Jose Gustavo
 *
 *  Based on: NMEA library
 *      URL: http://nmea.sourceforge.net
 *      Author: Tim (xtimor@gmail.com)
 *      Licence: http://www.gnu.org/licenses/lgpl.html
 *      $Id: gmath.c 17 2008-03-11 11:56:11Z xtimor $
 */

/**
 * \brief Convert NDEG (NMEA degree) to fractional degree
 */

double nmea_ndeg2degree(double val)
{
    double deg = ((int)(val / 100));
    val = deg + (val - deg * 100) / 60;
    return val;
}

