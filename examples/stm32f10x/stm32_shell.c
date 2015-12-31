/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  The stm32 shell example
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :  2015-11-24
 * @Update   :
 * 
 * @Notice   :    the end of the command must be 0x0A
 */

#ifdef WITH_CONTIKI_SHELL

#include "contiki.h"
#include "shell.h"
#include "serial-shell.h"

PROCESS(stm32_shell_process, "STM32 Contiki Shell");

extern void shell_blink_init(void);

PROCESS_THREAD(stm32_shell_process, ev, data)
{
    PROCESS_BEGIN();

    serial_shell_init();
		/* add the shell blink command */
    shell_blink_init();
    PROCESS_END();
}


#endif /* end of WITH_CONTIKI_SHELL */
