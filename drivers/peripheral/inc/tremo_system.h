/**
 ******************************************************************************
 * @file    tremo_system.h
 * @author  ASR Tremo Team
 * @version v1.6.3
 * @date    2022-12-09
 * @brief   This file contains the delay functions
 * @addtogroup Tremo_Drivers
 * @{
 * @defgroup SYSTEM
 * @{
 */

#ifndef __TREMO_SYSTEM_H
#define __TREMO_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tremo_cm4.h"


void system_reset(void);
int32_t system_get_chip_id(uint32_t* id);

#ifdef __cplusplus
}
#endif
#endif //__TREMO_SYSTEM_H

/**
 * @} 
 * @}
 */
