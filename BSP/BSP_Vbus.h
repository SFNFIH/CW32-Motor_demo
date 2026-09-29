#ifndef BSP_VBUS_H
#define BSP_VBUS_H

#include <stdint.h>

void BSP_Vbus_Init(void);
uint16_t BSP_Vbus_ReadRaw(void);
float BSP_Vbus_ReadVolt(void);

#endif
