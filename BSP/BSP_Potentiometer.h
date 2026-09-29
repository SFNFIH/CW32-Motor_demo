#ifndef BSP_POTENTIOMETER_H
#define BSP_POTENTIOMETER_H

#include "cw32l012_adc.h"

#ifdef __cplusplus
extern "C" {
#endif

void BSP_Potentiometer_Init(void);

uint16_t BSP_Potentiometer_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_POTENTIOMETER_H */