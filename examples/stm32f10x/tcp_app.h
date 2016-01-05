/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  TCP app header
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :  2016-1-4
 * @Update   :
 *
 */
#ifndef TCP_APP_H_
#define TCP_APP_H_

#include "uip.h"

typedef struct {
	uint8_t state;
	uint8_t *tptr;
	uint8_t tlen;
}tcp_sta_t;

#endif /* TCP_APP_H_ */


