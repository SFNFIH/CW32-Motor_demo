/**
  ******************************************************************************
  * @file    main.c
  * @author  AE Team
  * @version 1.0
  * @date    2024-08-15
  * @brief   
  *
  * @note    硬件平台：CW32L012_StartKit_V1.0
  *          
  *
  *****************************************************************************/
/*******************************************************************************
*
* 代码许可和免责信息
* 武汉芯源半导体有限公司授予您使用所有编程代码示例的非专属的版权许可，您可以由此
* 生成根据您的特定需要而定制的相似功能。根据不能被排除的任何法定保证，武汉芯源半
* 导体有限公司及其程序开发商和供应商对程序或技术支持（如果有）不提供任何明示或暗
* 含的保证或条件，包括但不限于暗含的有关适销性、适用于某种特定用途和非侵权的保证
* 或条件。
* 无论何种情形，武汉芯源半导体有限公司及其程序开发商或供应商均不对下列各项负责，
* 即使被告知其发生的可能性时，也是如此：数据的丢失或损坏；直接的、特别的、附带的
* 或间接的损害，或任何后果性经济损害；或利润、业务、收入、商誉或预期可节省金额的
* 损失。
* 某些司法辖区不允许对直接的、附带的或后果性的损害有任何的排除或限制，因此某些或
* 全部上述排除或限制可能并不适用于您。
*
*******************************************************************************/
/******************************************************************************
 * Include files
 ******************************************************************************/
#include "../inc/main.h"
#include "cw32l012_eau.h"

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/


/******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/******************************************************************************
 * Local variable definitions ('static')                                      *
 ******************************************************************************/

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief  Main function of project
 **
 ** \return uint32_t return value, if needed
 **
 ** 
 **
 ******************************************************************************/
char RamBuf[100];
int32_t main(void)
{
    int32_t dividend = -1000;    // 被除数（有符号）
    int32_t divisor = 7;         // 除数（有符号）
    int32_t quotient;
    int32_t remainder;

    // 初始化EAU
    EAU_Init();

    // 设置为有符号除法模式
    EAU_SetMode(EAU_MODE_SIGNED_DIV);

    // 开始运算
    EAU_StartOperation((uint32_t)dividend, (uint32_t)divisor);

    // 等待运算完成
    while (EAU_GetStatus() & EAU_STATUS_BUSY) {
        // 等待忙碌标志位清零
    }

    // 获取结果
    quotient = (int32_t)EAU_GetQuotient();
    remainder = (int32_t)EAU_GetRemainder();

    // 输出结果
    sprintf(RamBuf, "signed div operation:\n");
    sprintf(&RamBuf[25],"dividend:%d\n", dividend);
    sprintf(&RamBuf[45],"divisor:%d\n", divisor);
    sprintf(&RamBuf[60],"quotient:%d\n", quotient);
    sprintf(&RamBuf[80],"remainder:%d\n", remainder);
    
    while(1);    
}


/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
#ifdef  USE_FULL_ASSERT
 /**
   * @brief  Reports the name of the source file and the source line number
   *         where the assert_param error has occurred.
   * @param  file: pointer to the source file name
   * @param  line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number */
    printf("Wrong parameters value: file %s on line %d\r\n", file, line);
       /* USER CODE END 6 */
}
#endif
