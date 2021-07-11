/*
 * time.h
 *
 *  Created on: 27 de jan de 2021
 *      Author: Jose Gustavo
 *
 *  Based on: NMEA library
 *      URL: http://nmea.sourceforge.net
 *      Author: Tim (xtimor@gmail.com)
 *      Licence: http://www.gnu.org/licenses/lgpl.html
 *      $Id: time.h 4 2007-08-27 13:11:03Z xtimor $
 */

#ifndef LIB_GPS_NMEA_TIME_H_
#define LIB_GPS_NMEA_TIME_H_

/**
 * Date and time data
 * @see nmea_time_init
 */

typedef struct _nmeaTIME
{
    int     year;       /**< Years since 1900 */
    int     mon;        /**< Months since January - [0,11] */
    int     day;        /**< Day of the month - [1,31] */
    int     hour;       /**< Hours since midnight - [0,23] */
    int     min;        /**< Minutes after the hour - [0,59] */
    int     sec;        /**< Seconds after the minute - [0,59] */
    int     hsec;       /**< Hundredth part of second - [0,99] */

} nmeaTIME;

/**
 * \brief Init nmeaTIME structure
 */

void nmea_time_init(nmeaTIME *t);

#endif /* LIB_GPS_NMEA_TIME_H_ */
