#include <Servo.h>
class Sensor {
private:
  const int sensor[5] = {A0, A1, A2, A3, A4};
  const int threshold = 500;
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
        // 小于阈值，则黑线在灰度传感器下方，result为1
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

    if (cnt == 0 || cnt > 2) {
      flag = -1;
    }
    // 未检测到黑线或者有3个以上的黑线
    if (cnt == 1) {
      for (int i = 0; i < 5; i++) {
        if (sensor_result[i]) {
          position = i;
        }
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
        // 如果不相邻，则报错
      }
    }
  }

public:
  float getStatus() {
    read();
    if (flag == -1) {
      return -1;
    } else {
      return position;
    }
  }
};

class Motor {
private:
  // A组电机驱动
  const int A_PWM = 6; // 控制速度
  const int A_DIR = 7; // 控制方向
  // B组电机驱动
  const int B_PWM = 5; // 控制速度
  const int B_DIR = 4; // 控制方向

  const int servo_pwm_pin = 9;

  const int offset = 140; // 舵机的零点

  // 在这辆小车上，默认A：LOW，B：HIGH
  const int dir_A = 0;
  const int dir_B = 1;
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
    pinMode(B_PWM, OUTPUT); // 全部都设置为输出
    servo.attach(servo_pwm_pin);
  }
  void run_Motor(const int A_speed, const int B_speed, const int dir_A,
                 const int dir_B) {
    // 注意电机旋转方向,两者可能不一致
    // 转向时使AB电机转速不同以辅助转向
    A_Motor(dir_A, A_speed);
    B_Motor(dir_B, B_speed);
  }

  void turn_ctrl(const int Aspeed, const int Bspeed, const int angle,
                 int mode) {
    servo.write(offset + angle);
    // 在这辆车上，A是右轮，B是左轮
    switch (mode) {
    case 0:
      // 默认模式
      run_Motor(Aspeed, Bspeed, dir_A, dir_B);
      break;
    case 1:
      // 极限左转，左电机反转
      run_Motor(Aspeed, Bspeed, dir_A, !dir_B);
      break;
    case 2:
      // 极限右转，右电机反转
      run_Motor(Aspeed, Bspeed, !dir_A, dir_B);
      break;
    case 3:
      // 倒车(试验)
      run_Motor(Aspeed, Bspeed, !dir_A, !dir_B);
      break;
    }
  }

  void straight() {
    run_Motor(150, 150, dir_A, dir_B);
    servo.write(offset);
  }

  void stop() {
    run_Motor(0, 0, dir_A, dir_B);
    servo.write(offset);
  }
};

class Car : private Sensor, private Motor {
public:
  int last_status = 4;

  void start() { Motor::init(); }

  void tracing_adjust(int state) {
    // 分段调控函数，这里只考虑检测正确的情况
    const int R = -1;
    const int L = 1;
    // 12/25换了新舵机，好像角度不一样
    // A_Motor为右轮，B_Motor为左轮
    const int Aspeed_0to3[4] = {50, 20, 100, 130};
    const int Bspeed_0to3[4] = {140, 130, 140, 150};
    const int angle_0to3[4] = {68, 54, 38, 28};
    // 上面是右转，角度需要大一点，但是最大别超过54了，以免损伤舵机

    const int Aspeed_5to8[4] = {105, 120, 135, 155};
    const int Bspeed_5to8[4] = {95, 100, 120, 130};
    const int angle_5to8[4] = {48, 38, 28, 18};
    const int STR[2] = {160, 160};
    // 电机最大速度可以到250
    switch (state) {
    case 0:
      // 0,黑线在左，需右转
      // 极限转弯，右轮反转
      Motor::turn_ctrl(Aspeed_0to3[0], Bspeed_0to3[0], R * angle_0to3[0], 2);
      last_status = 0;
      break;
    case 1:
      // 0.5
      Motor::turn_ctrl(Aspeed_0to3[1], Bspeed_0to3[1], R * angle_0to3[1], 0);
      last_status = 1;
      break;
    case 2:
      // 1
      Motor::turn_ctrl(Aspeed_0to3[2], Bspeed_0to3[2], R * angle_0to3[2], 0);
      last_status = 2;
      break;
    case 3:
      // 1.5
      Motor::turn_ctrl(Aspeed_0to3[3], Bspeed_0to3[3], R * angle_0to3[3], 0);
      last_status = 3;
      break;
    case 4:
      // 2,黑线在中
      Motor::turn_ctrl(STR[0], STR[1], 2, 0);
      last_status = 4;
      break;
    case 5:
      // 2.5,黑线在右，需左转
      Motor::turn_ctrl(Aspeed_5to8[3], Bspeed_5to8[3], L * angle_5to8[3], 0);
      last_status = 5;
      break;
    case 6:
      // 3,黑线在右，需左转
      Motor::turn_ctrl(Aspeed_5to8[2], Bspeed_5to8[2], L * angle_5to8[2], 0);
      last_status = 6;
      break;
    case 7:
      // 3.5,黑线在右，需左转
      Motor::turn_ctrl(Aspeed_5to8[1], Bspeed_5to8[1], L * angle_5to8[1], 0);
      last_status = 7;
      break;
    case 8:
      // 4,黑线在右，需左转
      // 12/25测试后发现极限左转不需要反转
      Motor::turn_ctrl(Aspeed_5to8[0], Bspeed_5to8[0], L * angle_5to8[0], 0);
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
      // 处理错误情况
      tracing_adjust(last_status);
    }
  }

  void run_without_tracing() { Motor::straight(); }

  void servo_test() {
    const int L = -1;
    const int R = 1;
    Motor::turn_ctrl(75, 0, L * 20, 0);
    // 向右转，输入-参数
    delay(1500);
    Motor::turn_ctrl(0, 75, R * 20, 0);
    // 向左转，输入+参数
    delay(3000);
  }

  void motor_direction() { Motor::turn_ctrl(120, 130, 0, 1); }
};