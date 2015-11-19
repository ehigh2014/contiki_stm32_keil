/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header:
 * @Author :  luochao
 * @Email  :  luochao@everhigh.com.cn
 * @Date :
 * @Update:
 *
 */

#include "stm32f10x.h"
#include "stdint.h"
#include "stdio.h"
#include "stm32f10x_gpio.h"

#include "sys/autostart.h"
#include "debug-uart.h"
#include "clock.h"
#include "etimer.h"
#include "sys/process.h"

#ifdef WITH_LED_BLINK
extern struct process blink_process;
#endif

uint32_t idle_count = 0;

int
main()
{
	//dubug uart init
	dbg_setup_uart();
	clock_init();
	//process init
	process_init();
	process_start(&etimer_process, NULL);

	/* with keil, can't use the AUTOSTART_PROCESSES to add the exmaple or it will be error
	 * So in this project, start the process manual.
	 */
	#ifdef WITH_LED_BLINK
	process_start(&blink_process, NULL);
	#endif
	while(1)
	{
			do{}
			while(process_run()>0);
			idle_count++;
	}
}

