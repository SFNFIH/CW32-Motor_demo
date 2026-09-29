/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    interrupts_cw32l012.h
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 CW.
  * All rights reserved.</center></h2>
  *
  *
 ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INTERRUPTS_CW32L012_H
#define __INTERRUPTS_CW32L012_H

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */


/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */


/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */


/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */


/* Exported functions prototypes ---------------------------------------------*/

extern void WDT_IRQHandlerCallBack(void);          
extern void LVD_IRQHandlerCallBack(void);          
extern void RTC_IRQHandlerCallBack(void);          
extern void FLASHRAM_IRQHandlerCallBack(void);     
extern void SYSCTRL_IRQHandlerCallBack(void);      
extern void GPIOA_IRQHandlerCallBack(void);        
extern void GPIOB_IRQHandlerCallBack(void);        
extern void GPIOC_IRQHandlerCallBack(void);        
extern void GPIOF_IRQHandlerCallBack(void);        
extern void DMACH12_IRQHandlerCallBack(void);      
extern void DMACH34_IRQHandlerCallBack(void);      
extern void CORDIC_IRQHandlerCallBack(void);       
extern void ADC1_IRQHandlerCallBack(void);         
extern void ATIM_IRQHandlerCallBack(void);         
extern void VC13_IRQHandlerCallBack(void);         
extern void VC24_IRQHandlerCallBack(void);         
extern void GTIM1_IRQHandlerCallBack(void);        
extern void GTIM2_IRQHandlerCallBack(void);        
extern void GTIM34_IRQHandlerCallBack(void);       
extern void LPTIM_IRQHandlerCallBack(void);    
extern void BTIM1_IRQHandlerCallBack(void);        
extern void BTIM2_IRQHandlerCallBack(void);        
extern void BTIM3_HALLTIM_IRQHandlerCallBack(void);
extern void I2C1_IRQHandlerCallBack(void);         
extern void I2C2_IRQHandlerCallBack(void);         
extern void SPI1_IRQHandlerCallBack(void);         
extern void SPI23_IRQHandlerCallBack(void);        
extern void UART1_IRQHandlerCallBack(void);        
extern void UART2_IRQHandlerCallBack(void);        
extern void UART3_IRQHandlerCallBack(void);        
extern void ADC2_DAC_IRQHandlerCallBack(void);     
extern void CLKFAULT_IRQHandlerCallBack(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */


#ifdef __cplusplus
}
#endif

#endif /* __INTERRUPTS_CW32L012_H */

/************************ (C) COPYRIGHT CW *****END OF FILE****/
