#ifndef HUIDU_H
#define HUIDU_H

#include "ti_msp_dl_config.h"

/* 五路灰度：L1 L2 M R1 R2，每个元素 0/1 */
extern uint8_t huidu_value[5];

void huidu_get_value(void);
void adjust_motor(void);

#endif // !HUIDU_H
