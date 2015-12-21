/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  An example of shell range
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :  2015-11-24
 * @Update   :
 *
 */

#include "contiki.h"
#include "shell.h"

#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"
#include "SR04.h"

PROCESS(shell_range_process, "shell range");

SHELL_COMMAND(range_command, "range", "range : range a time", &shell_range_process);

PROCESS_THREAD(shell_range_process, ev, data)
{
    PROCESS_BEGIN();
    while(1)
    {
        static struct etimer et;
        double distance = 0;

        etimer_set(&et, CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        distance = SR04_Ranging();
        printf("Distance : %.2f (cm)\n", distance);
    }
    PROCESS_END();
}

void
shell_range_init(void)
{
    SR04_Init();
    shell_register_command(&range_command);
}



