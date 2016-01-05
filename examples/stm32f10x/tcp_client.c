/*
 *  Copyright (c) 2015, Everhigh Tech. Ltd. Co.
 *  All rights reserved
 *
 * @Header   :  A Tcp client test
 * @Author   :  luochao
 * @Email    :  luochao@everhigh.com.cn
 * @Date     :  2016-1-4
 * @Update   :
 *
 */
#include "uip.h"
#include "tcp_app.h"

void
tcp_client_call(void)
{
	tcp_sta_t *s = (tcp_sta_t *)&uip_conn->appstate;
	
}