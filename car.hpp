#include <Servo.h>
class Sensor {
private:
  const int sensor[5] = { A0, A1, A2, A3, A4 };
  const int threshold = 450;
  int sensorReading[5];
  int flag = 0;
  int sensor_result[5];
  float position;
  void read() {
    int cnt = 0;
    for (int i = 0; i < 5; i++) {
      sensorReading[i] = analogRead(sensor[i]);
      Serial.print(sensorReading[i]);
      Serial.print('-');

      if (sensorReading[i] < threshold) {
        //小于阈值，则黑线在灰度传感器下方，result为1
        sensor_result[i] = 1;
        cnt++;
      } else {
        sensor_result[i] = 0;
      }
    }
    Serial.println();

    for (int i = 0; i < 5; i++) {
      Serial.print(sensor_result[i]);
      Serial.print('-');
    }
    Serial.println();

    if (cnt == 0 || cnt > 2) { flag = -1; }
    //未检测到黑线或者有3个以上的黑线
    if (cnt == 1) {
      for (int i = 0; i < 5; i++) {
        if (sensor_result[i]) { position = i; }
      }
      flag = 1;
    }
    if (cnt == 2) {
      bool flag2 = 0;
      for (int i = 0; i < 4; i++) {
        if (sensor_result[i] == sensor_result[i + 1] && sensor_result[i] == 1) {
          flag2 = 1;
          position = i + 0.5;
        }
      }
      if (flag2) {
        flag = 1;
      } else {
        flag = -1;
        //如果不相邻，则报错
      }
    }
  }
public:
  float getStatus() {
    read();
    if (flag == -1)
      return -1;
    else
      return position;
  }
};

class Motor {
private:
  // A组电机驱动
  const int A_PWM = 6;  // 控制速度
  const int A_DIR = 7;  // 控制方向
  // B组电机驱动
  const int B_PWM = 5;  // 控制速度
  const int B_DIR = 4;  // 控制方向

  const int servo_pwm_pin = 9;

  const int offset = 83;  // 舵机的零点

  const int straight_speed = 150;
  const int turn_speed = 95;
  const int sharp_turn_speed = 70;

  Servo servo;

  // A组电机驱动控制函数
  void A_Motor(int dir, int speed) {
    digitalWrite(A_DIR, dir);
    analogWrite(A_PWM, speed);
  }

  // B组电机驱动控制函数
  void B_Motor(int dir, int speed) {
    digitalWrite(B_DIR, dir);
    analogWrite(B_PWM, speed);
  }

public:
  void init() {
    pinMode(A_DIR, OUTPUT);
    pinMode(A_PWM, OUTPUT);
    pinMode(B_DIR, OUTPUT);
    pinMode(B_PWM, OUTPUT);  // 全部都设置为输出
    servo.attach(servo_pwm_pin);
  }
  void run_Motor(const int A_speed, const int B_speed) {
    //注意电机旋转方向,两者可能不一致
    //转向时使AB电机转速不同以辅助转向
    A_Motor(LOW, A_speed);
    B_Motor(HIGH, B_speed);
  }

  void angle_loop(float times) {
    float angle = 0;
    const float angle_start = 60;
    const float angle_end = 140;
    for (int i = 0; i <= times; i++) {
      angle = angle_start + i * (angle_end - angle_start) / times;
      servo.write(angle);
      Serial.print("angle");
      Serial.print(i);
      Serial.print("=");
      Serial.print(angle);
      Serial.println();
      delay(500);
    }
  }

  void turn_ctrl(const int A_speed, const int B_speed, const int angle) {
    run_Motor(A_speed, B_speed);
    servo.write(offset + angle);
  }

  void straight() {
    run_Motor(straight_speed, straight_speed);
    servo.write(offset);
  }

  void stop() {
    run_Motor(0, 0);
    servo.write(offset);
  }
};

class Car : private Sensor, private Motor {
public:
  int last_status = 4;

  void start() {
    Motor::init();
  }

  void tracing_adjust(int state) {
    //分段调控
    //此函数中只考虑检测正确的情况
    const int L = -1;
    const int R = 1;
    //右转，角度需要大一点
    const int A_speed_0to3[4] = { 10, 85, 103, 115 };
    //A右轮，B左轮
    const int B_speed_0to3[4] = { 95, 100, 107, 115 };
    const int angle_0to3[4] = { 58, 51, 43, 33 };
    //左转，角度需要大一点
    const int A_speed_5to8[4] = { 85, 100, 107, 115 };
    const int B_speed_5to8[4] = { 45, 85, 103, 115 };
    const int angle_5to8[4] = { 39, 29, 19, 9 };
    //直行时，调试时可以用来测试差速，调试结束后记得消除差速
    const int STR[2] = { 123, 123 };
    switch (state) {
      case 0:
        //0,黑线在右，需左转
        //其实是黑线在左，需要右转
        //这里把速度也调慢了
        Motor::turn_ctrl(A_speed_0to3[0], B_speed_0to3[0], L * angle_0to3[0]);
        last_status = 0;
        break;
      case 1:
        //0.5,黑线在右，需左转
        Motor::turn_ctrl(A_speed_0to3[1], B_speed_0to3[1], L * angle_0to3[1]);
        last_status = 1;
        break;
      case 2:
        //1,黑线在右，需左转
        Motor::turn_ctrl(A_speed_0to3[2], B_speed_0to3[2], L * angle_0to3[2]);
        last_status = 2;
        break;
      case 3:
        //1.5,黑线在右，需左转
        Motor::turn_ctrl(A_speed_0to3[3], B_speed_0to3[3], L * angle_0to3[3]);
        last_status = 3;
        break;
      case 4:
        //2,黑线在中
        Motor::turn_ctrl(STR[0], STR[1], 2);
        last_status = 4;
        break;
      case 5:
        //2.5,黑线在左，需右转
        Motor::turn_ctrl(A_speed_5to8[3], B_speed_5to8[3], R * angle_5to8[3]);
        last_status = 5;
        break;
      case 6:
        //3,黑线在左，需右转
        Motor::turn_ctrl(A_speed_5to8[2], B_speed_5to8[2], R * angle_5to8[2]);
        last_status = 6;
        break;
      case 7:
        //3.5,黑线在左，需右转
        Motor::turn_ctrl(A_speed_5to8[1], B_speed_5to8[1], R * angle_5to8[1]);
        last_status = 7;
        break;
      case 8:
        //4,黑线在左，需右转
        Motor::turn_ctrl(A_speed_5to8[0], B_speed_5to8[0], R * angle_5to8[0]);
        last_status = 8;
        break;
    }
  }

  void run_with_tracing() {
    float status = Sensor::getStatus();
    Serial.println(status);
    int state = static_cast<int>(2 * status);
    if (state != -2) {
      tracing_adjust(state);
    } else {
      //处理错误情况
      tracing_adjust(last_status);
    }
  }

  void run_without_tracing() {
    Motor::straight();
  }

  void servo_test() {
    const int L = -1;
    const int R = 1;
    Motor::turn_ctrl(75, 75, L * 25);
    //向右转，输入-参数
    delay(1500);
    Motor::turn_ctrl(75, 75, R * 115);
    //向左转，输入+参数
    delay(3000);
  }

  void find_offset() {
    angle_loop(10);
  }

  void motor_direction() {
    Motor::turn_ctrl(120, 30, 0);
    //120右轮
    //30左轮
  }
};