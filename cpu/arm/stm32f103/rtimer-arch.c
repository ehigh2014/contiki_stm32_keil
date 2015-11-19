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
 * \file 
 *     RTIMER for STM32F10x ( not implemented )
 */

#include "sys/rtimer.h"
#include "sys/clock.h"
#include "contiki-conf.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static volatile uint32_t compare_timer;
static volatile uint32_t last_expired_timer;

void
rtimer_arch_run_next(uint32_t u32DeviceId, uint32_t u32ItemBitmap)
{
}

void
rtimer_arch_init(void)
{
}

rtimer_clock_t
rtimer_arch_now(void)
{
	return (rtimer_clock_t)0;
}

void 
rtimer_arch_schedule(rtimer_clock_t t)
{

}

rtimer_clock_t
rtimer_arch_get_time_until_next_wakeup(void)
{
	return (rtimer_clock_t)-1;
}


