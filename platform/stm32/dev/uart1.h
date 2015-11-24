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

#ifndef UART1_H_
#define UART1_H_

#include "contiki-conf.h"

void uart1_set_input(int (* input)(unsigned char c));
	
#endif /* UART1_H_ */

