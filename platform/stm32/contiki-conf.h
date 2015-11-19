/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header:
 * @Author : luochao
 * @Email  : luochao@everhigh.com.cn
 * @Date :
 * @Update:
 *
 */

#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>

#define CLOCK_CONF_SECOND 1000

#define CCIF
#define CLIF

typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

#endif /* CONTIKI_CONF_H */
