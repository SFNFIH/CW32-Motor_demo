/**
 * @file main.c
 * @author WHXY
 * @brief
 * @version 0.1
 * @date 2024-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */
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
#include "cw32l012_cordic.h"
#include "cw32l012_sysctrl.h"

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
char RamBuf[100] = {0};
int main()
{    
    cordic_init_t init = {
        .func = CORDIC_FUNC_SQRT, // 选择平方根函数
        .scale = 0,
        .format = CORDIC_FORMAT_Q1_31,
        .iter = CORDIC_ITER_32,
        .comp = 1,
        .ie = 0,
        .dmaeoc = 0,
        .dmaidle = 0
    };
    
    memset(RamBuf, 0, 100);

    // 初始化CORDIC模块
    CORDIC_Init(&init);

    // 输入X值（q1.31格式）
    int32_t x = float_to_q1_31(0.4); // 0.4
    CW_CORDIC->X = x;       // 写入X寄存器启动运算

    while (!CORDIC_GetStatus().eoc);

    int32_t result = CW_CORDIC->X; // 平方根结果在X寄存器
    sprintf(RamBuf,"sqrt(0.4) = %d (q1.31 format)\n", result);    // 在RamBuf中看结果q1.31格式
    sprintf(&RamBuf[47],"sqrt(0.4) = %f \n", q1_31_to_float(result));    // 在RamBuf中看结果float格式
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

