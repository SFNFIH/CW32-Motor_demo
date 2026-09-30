示例目的：
        演示CORDIC的硬件算法相位角atan2。

硬件资源：
          1. CW32L012 StartKit
          2. 时钟HSI
          3. 系统时钟设置为HSI时钟默认24分频，4MHz， PCLK、HCLK不分频，PCLK=HCLK=SysClk=4MHz

演示说明：
           计算结果将在RAM（0x20000200）中显示，设置为Ascii可查看

使用说明：
+ EWARM
          1. 打开project.eww文件
          2. 编译所有文件：Project->Rebuild all
          3. 载入工程镜像：Project->Debug
          4. 运行程序：Debug->Go(F5)

+ MDK-ARM
          1. 打开project.uvproj文件
          2. 编译所有文件：Project->Rebuild all target files
          3. 载入工程镜像：Debug->Start/Stop Debug Session
          4. 运行程序：Debug->Run(F5)
