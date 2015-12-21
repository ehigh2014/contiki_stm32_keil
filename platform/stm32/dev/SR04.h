/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :
 * @Update   :
 *
 */

#ifndef SR04_H_
#define SR04_H_

#include "stm32f10x.h"
#include "stdint.h"

#include "sys/clock.h"
#include "stdio.h"

void SR04_Init();
double SR04_Ranging();

#endif /* SR04_H_ */

