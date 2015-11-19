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
 * \file  MT arch for STM32F10x ( not implemented )
 */

#include "mtarch.h"

void
mtarch_init(void)
{
}

void 
mtarch_remove(void)
{
}

void
mtarch_start(struct mtarch_thread *thread,
             void (*function)(void *data),
             void *data)
{
}

void
mtarch_yield(void)
{
}
void
mtarch_exec(struct mtarch_thread *thread)
{
}

void
mtarch_stop(struct mtarch_thread *thread)
{
}

void
mtarch_pstart(void)
{
}

void
mtarch_pstop(void)
{
}




