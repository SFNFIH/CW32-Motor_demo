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
char RamBuf[100];
int main()
{    
    cordic_init_t init = {
        .func = CORDIC_FUNC_COS,  // 选择余弦函数
        .scale = 0,              // 不使用扩展范围
        .format = CORDIC_FORMAT_Q1_15, // 使用q1.15格式
        .iter = CORDIC_ITER_20,  // 迭代次数
        .comp = 1,               // 硬件补偿伸缩因子
        .ie = 0,                 // 禁用中断
        .dmaeoc = 0,             // 禁用DMA
        .dmaidle = 0             // 禁用DMA空闲
    };
    
    memset(RamBuf, 0, 100);

    // 初始化CORDIC模块
    CORDIC_Init(&init);

    // 输入角度（以pi为单位，q1.15格式）
    int32_t angle = float_to_q1_15(0.25);   // 0.25 Q1.15: 0x2000, 即：PI/4
    CW_CORDIC->Z = angle; // 写入Z寄存器启动运算
    
    // 等待运算完成
    while (!CORDIC_GetStatus().eoc);  //运算完成标志硬件置1，读取运算结果硬件清0

    // 读取结果
    int32_t result = CW_CORDIC->X; // 余弦结果在X寄存器
    sprintf(RamBuf, "cos(0.25 * pi) = %d (q1.15 format)\n", result&0xFFFF);    // 在RamBuf中看结果q1.15格式
    sprintf(&RamBuf[59], "cos(0.25 * pi) = %f\n", q1_15_to_float(result&0xFFFF));    // 在RamBuf中看结果
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

