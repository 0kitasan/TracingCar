# TracingCar
工程实践课程大作业：巡线小车

由于使用面向对象的方法编写程序，故使用mermaid中的classDiagram构建程序框图。
请注意，该框图只展示了大致的逻辑，变量和函数名也并非一一对应。

若想具体了解可以直接查看代码(即car.hpp)，内部附有详细注释。

以下是控制程序逻辑框图：

``` mermaid
classDiagram
    Sensor --|> Car
    Motor --|> Car
    class Sensor{
        -const int sensor[5]
        -int sensorReading[5]
        -int sensor_result[5]
        -const int threshod //灰度阈值
        -void read() 读数
        +float getStatus() 输出状态以判断小车位置
    }
    class Motor{
        -const int PWM_and_Dir
        -const int offset //设置舵机零点
        
        -void A_and_B_Motor()
        +staright() 直行
        +turn_ctrl() 提供控制舵机和后轮的接口，\n可分别控制两后轮，并提供后轮方向选择
    }
    class Car{
        -int last_status //记录小车上一次位置状态
        -const int Speed_and_Dir_Param[ ] 
        //小车速度参数，需要重点调整
        +void tracing_adjust(int state) 控制小车转向
        +void servo_test() 调试舵机，用于调整其零点\n//下面两个函数将在ino中被直接调用
        +void run_without_tracing() 直线模式
        +void run_with_tracing() 巡线模式
    }
```


```mermaid
graph TD;
    A[run_with_tracing]
    B[getStatus]
    I[turn_ctrl+tracing_adjust]
    C[黑线极左\n右转需打死]
    D[黑线较左\n右转幅度需较大]
    E[黑线偏左\n右转]
    F[黑线稍左\n稍微右转即可]
    G[黑线在中\n直行]
    H[使小车维持上一次运动状态]
    A -->|调用|I
    I -->|封装|A
    I -->|调用|B
    B -->|调控|I
    B -->|1 0 0 0 0\n黑线在最右面\n传感器下方|C
    B -->|1 1 0 0 0\n黑线在相邻\n传感器下方|D
    B -->|0 1 0 0 0\n黑线在右二\n传感器下方|E
    B -->|0 1 1 0 0|F
    B -->|0 0 1 0 0|G
    B -->|0 0 0 0 0\n错误情况|H
    C -->|传递状态0|H
    D -->|传递状态1|H
    E -->|传递状态2|H
    F -->|传递状态3|H
    G -->|传递状态4|H
    H ==>|调控|I
```