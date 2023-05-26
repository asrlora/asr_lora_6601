/**
 ******************************************************************************
 * @file    tremo_delay.h
 * @author  ASR Tremo Team
 * @version v2.0.0
 * @date    2023-05-19
 * @brief   This file contains the delay functions
 * @addtogroup Tremo_Drivers
 * @{
 * @defgroup DELAY
 * @{
 */

#ifndef __TREMO_DELAY_H
#define __TREMO_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tremo_cm4.h"


void delay_init(void);
void delay_us(uint32_t nus);
void delay_ms(uint32_t nms);

#ifdef __cplusplus
}
#endif
#endif //__TREMO_DELAY_H

/**
 * @} 
 * @}
 */
