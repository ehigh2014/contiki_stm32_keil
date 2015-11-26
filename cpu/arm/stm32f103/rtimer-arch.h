/**
 * \file
 *         Header file for the STM32F103-specific rtimer code
 * \author
 *         Simon Berg <ksb@users.sourceforge.net>
 *
 * \update luochao <luochao@everhigh.com.cn>
 */

#ifndef RTIMER_ARCH_H_
#define RTIMER_ARCH_H_

#include "sys/rtimer.h"

#define RTIMER_ARCH_SECOND  10000

rtimer_clock_t rtimer_arch_now(void);

/* enable/disable the rtimer irq */
void rtimer_arch_disable_irq(void);
void rtimer_arch_enable_irq(void);

#endif /* RTIMER_ARCH_H_ */
