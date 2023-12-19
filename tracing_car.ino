#include "car.hpp"
Car car;
void setup()
{
    car.start();
    Serial.begin(9600); // 打开串口，设置波特率为9600 bps
}

void loop()
{
    //car.run_without_tracing();
    car.run_with_tracing();
    //car.motor_direction();
    //car.find_offset();
    //car.servo_test();
    //Serial.println("");
    delay(5);
}
