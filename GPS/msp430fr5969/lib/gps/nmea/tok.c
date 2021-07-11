/*
 * tok.c
 *
 *  Created on: 27 de jan de 2021
 *      Author: Jose Gustavo
 *
 *  Based on: NMEA library
 *      URL: http://nmea.sourceforge.net
 *      Author: Tim (xtimor@gmail.com)
 *      Licence: http://www.gnu.org/licenses/lgpl.html
 *      $Id: tok.c 17 2008-03-11 11:56:11Z xtimor $
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <lib/gps/nmea/gmath.h>
#include <lib/gps/nmea/tok.h>
#include <string.h>
#include <limits.h>
#include <math.h>


#define NMEA_CONVSTR_BUF (256)

/**
 * \brief Calculate control sum of binary buffer
 */

unsigned short int nmea_calc_crc(char *buff, unsigned short int buff_sz)
{
    unsigned short int chsum = 0, it;

    for(it = 1; it < buff_sz; ++it)
        chsum ^= (unsigned short int)buff[it];

    return chsum;
}

/**
 * \brief Convert string to fraction number
 */

double nmea_atof(char *str, int str_sz)
{
    char *tmp_ptr;
    char buff[NMEA_CONVSTR_BUF];
    double res = 0;

    if(str_sz < NMEA_CONVSTR_BUF)
    {
        memcpy(&buff[0], str, str_sz);
        buff[str_sz] = '\0';
        res = strtod(&buff[0], &tmp_ptr);
    }

    return res;
}

/**
 * \brief Convert string to number
 */
int nmea_atoi(char *str, int str_sz, int radix)
{
    char *tmp_ptr;
    char buff[NMEA_CONVSTR_BUF];
    int res = 0;

    if(str_sz < NMEA_CONVSTR_BUF)
    {
        memcpy(&buff[0], str, str_sz);
        buff[str_sz] = '\0';
        res = strtol(&buff[0], &tmp_ptr, radix);
    }

    return res;
}

char* checksum(char *buff){
    char *bkp_aux, *chsum, chsum_calculed[5];

    bkp_aux = malloc(strlen(buff) + 1);
    strcpy(bkp_aux, buff);

    chsum = strrchr(bkp_aux, '*');

    if(chsum == NULL){
        free(bkp_aux);
        return NULL;
    }

    memmove(&chsum[0], &chsum[1], strlen(chsum));
    sprintf(chsum_calculed, "%X", nmea_calc_crc(buff, strcspn(buff, "*")));

    free(bkp_aux);
    free(chsum);
    return chsum_calculed;
}

/**
 * \brief Analysis string GPRMC
 */

unsigned short int nmea_scanf(char *buff, nmeaGPRMC *pack)
{
    char *chsum, *bkp_aux, *token, *data;
    char chsum_calculed[5], gprmc_info[11][15], bkp_aux2[50];
    unsigned short int index;
    int date;
    double fractpart, intpart, double_bkp;

    bkp_aux = malloc(strlen(buff) + 1);
    strcpy(bkp_aux, buff);

    /** Verify sentence checksum **/
    chsum = strrchr(bkp_aux, '*');

    if(chsum == NULL){
        free(bkp_aux);
        return 0;
    }

    memmove(&chsum[0], &chsum[1], strlen(chsum));
    sprintf(chsum_calculed, "%X", nmea_calc_crc(buff, strcspn(buff, "*")));

    if(strcmp(chsum, chsum_calculed) != 0){
        free(bkp_aux);
        return 0;
    }

    /* Split data*/
    memset(bkp_aux, 0, sizeof(bkp_aux));
    strcpy(bkp_aux, buff);
    token = strchr(bkp_aux, ',');
    index = 0;

    while(token != NULL) {
        memmove(&token[0], &token[1], strlen(token));

        if(token[0] != ','){
            memset(&bkp_aux2, 0, sizeof(bkp_aux2));
            strcpy(bkp_aux2, token);
            data = strtok(bkp_aux2, ",");
            strcpy(gprmc_info[index], data);
        }else{
            strcpy(gprmc_info[index], ",");
        }

        /* Next index */
        index++;
        token = strchr(bkp_aux, ',');
    }

    /* Parse the information */
    /* Parse fix time */
    if(strlen(gprmc_info[0]) >= 8 && strrchr(gprmc_info[0], '.') != NULL){
        double_bkp = nmea_atof(gprmc_info[0], strlen(gprmc_info[0]));
        fractpart = modf(double_bkp, &intpart);
        pack->utc.hour = (int)(intpart / 10000);
        pack->utc.min = (int)(intpart / 100) % 100;
        pack->utc.sec = (int)(intpart) % 100;
        pack->utc.hsec = (int)(fractpart * 100);
    }else{
        pack->utc.hour = 0;
        pack->utc.min = 0;
        pack->utc.sec = 0;
        pack->utc.hsec = 0;
    }

    /* Parse status (active/fixed or void) */
    if(gprmc_info[1] != NULL && (strcmp(gprmc_info[1], "A") == 0 || strcmp(gprmc_info[1], "a") == 0)){
        pack->status = 'A';
    }else if(gprmc_info[1] != NULL && (strcmp(gprmc_info[1], "V") == 0 || strcmp(gprmc_info[1], "v") == 0)){
        pack->status = 'V';
    }else{
        free(token);
        free(bkp_aux);
        return 0;
    }

    /* Parse latitude and longitude */
    if(gprmc_info[2] != NULL){
        pack->lat = nmea_ndeg2degree(nmea_atof(gprmc_info[2], strlen(gprmc_info[2])));

        if(gprmc_info[3] != NULL && (strcmp(gprmc_info[3], "S") == 0 || strcmp(gprmc_info[3], "s") == 0)){
            pack->ns = 'S';
            if(pack->lat != 0.0)
                pack->lat *= -1.0;
        }else if(gprmc_info[3] != NULL && (strcmp(gprmc_info[3], "N") == 0 || strcmp(gprmc_info[3], "n") == 0)){
            pack->ns = 'N';
        }else{
            free(token);
            free(bkp_aux);
            return 0;
        }

    }else{
        pack->lat = 0.0;
    }

    if(gprmc_info[4] != NULL){
        pack->lon = nmea_ndeg2degree(nmea_atof(gprmc_info[4], strlen(gprmc_info[4])));

        if(gprmc_info[5] != NULL && (strcmp(gprmc_info[5], "W") == 0 || strcmp(gprmc_info[5], "w") == 0)){
            pack->ew = 'W';
            if(pack->lon != 0.0)
                pack->lon *= -1.0;
        }else if(gprmc_info[5] != NULL && (strcmp(gprmc_info[5], "E") == 0 || strcmp(gprmc_info[5], "e") == 0)){
            pack->ew = 'E';
        }else{
            free(token);
            free(bkp_aux);
            return 0;
        }

    }else{
        pack->lon = 0.0;
    }

    /* Parse out speed and other simple numeric values */
    if(gprmc_info[6] != NULL){
        pack->speed = nmea_atof(gprmc_info[6], strlen(gprmc_info[6]));
    }else{
        pack->speed = 0.0;
    }

    if(gprmc_info[7] != NULL){
        pack->direction = nmea_atof(gprmc_info[7], strlen(gprmc_info[7]));
    }else{
        pack->direction = 0.0;
    }

    /* Parse fix date */
    if(strlen(gprmc_info[8]) == 6){
        date = nmea_atoi(gprmc_info[8], strlen(gprmc_info[8]), 10);
        pack->utc.day = (int)(date / 10000);
        pack->utc.mon = (int)(date / 100) % 100;
        pack->utc.year = (int)(date % 100 + 2000);
    }else{
        pack->utc.day = 0;
        pack->utc.mon = 0;
        pack->utc.year = 0;
    }

    /* Parse declination in degrees */
    if(gprmc_info[9] != NULL && strrchr(gprmc_info[9], '.') != NULL){
        pack->declination = nmea_atof(gprmc_info[9], strlen(gprmc_info[9]));
    }else{
        pack->declination = 0;
    }

    /* Parse declination East or West */
    if(gprmc_info[10] != NULL){
        if(gprmc_info[10][0] == 'W' || gprmc_info[10][0] == 'w'){
            pack->declin_ew = 'W';
        }else if(gprmc_info[10][0] == 'E' || gprmc_info[10][0] == 'e'){
            pack->declin_ew = 'E';
        }else{
            free(token);
            free(bkp_aux);
            return 0;
        }
    }

    free(token);
    free(bkp_aux);

    return 1;
}

