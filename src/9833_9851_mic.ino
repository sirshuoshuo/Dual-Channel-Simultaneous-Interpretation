#include <AD9851.h>
#include <SPI.h>
#include <MD_AD9833.h>
MD_AD9833 AD(/*DATA*/5, /*CLK*/4, /*FSYNC*/3);
/* Sweep details */
#define CENTRE        48500000UL  /* Hz */ // 载波频率 48.5MHz
#define DEVIATION     25000      /* Deviation in Hz (above and below) */ // 偏移量 25KHz
#define STEP          0      /* Step in Hz. Set to 0 for constant frequency */
#define RATE          100       /* ms between steps */ // 两次更新之间的时间间隔
#define V_IN A0  //用于收集 和 语音信号的引脚
#define V_IN1 A1  //用于收集语音信号的引脚
#define V_IN2 A2  //第二个收集语音信号

#define CALIBRATION   5700        /* Calibration for my test module; it runs fast */

#define AD9851_FQ_UD_PIN      2
#define AD9851_RESET_PIN      3

//两路语音信号的输出
#define V_OUT1 9
#define V_OUT2 10
// And MOSI=11, SCK=13

class MyAD9851 : public AD9851<AD9851_RESET_PIN, AD9851_FQ_UD_PIN> {};
MyAD9851 dds;
unsigned long next_update;        /* The millis() value for the next update */

void setup() {
  Serial.begin(38400);
  pinMode(V_IN,INPUT);
  pinMode(V_IN1, INPUT);
  pinMode(V_OUT1, OUTPUT);
  pinMode(V_IN2, INPUT);
  pinMode(V_OUT2, OUTPUT);
  AD.begin();
  AD.setMode(MD_AD9833::MODE_SINE);  // 选择正弦波模式
  AD.setFrequency(MD_AD9833::CHAN_0, 10000);  // 定义信号输出频率为10kH
  while (!Serial && (millis() <= 1000));
  next_update = millis();
}

void set_modulated_frequency(float X_IN)
{
  // 将 -1 到 1 的输入信号映射到调制幅度的范围 -128 到 +128
  //short modulation_amplitude = static_cast<short>((X_IN - 128) * 128 / 255); // 这里的转换可能导致信息丢失
  short modulation_amplitude = static_cast<short>(X_IN * 128);
  // 将调制幅度限制在 -128 到 +128 的范围内
  modulation_amplitude = constrain(modulation_amplitude, -128, 128);

  uint32_t centre_delta = dds.frequencyDelta(CENTRE);  // Calculate delta for carrier frequency
  uint32_t deviation_delta = dds.frequencyDelta(DEVIATION);  // Calculate delta for deviation

  // This deviation calculation will not overflow up to 700KHz deviation
  dds.setDelta(centre_delta+(modulation_amplitude*deviation_delta/128));
}



void loop() {
  int X_IN1;
  int X_IN2;
  X_IN1 = analogRead(V_IN1);
  X_IN2 = analogRead(V_IN2);
  Serial.print("第一路：");
  Serial.println(X_IN1);
  Serial.print("第二路：");
  Serial.println(X_IN2);
  
  // 将读取到的模拟信号值（0-1023）映射到PWM范围（0-255），并输出到V_OUT
  analogWrite(V_OUT1, X_IN1 / 4);  //输出语音信号
  analogWrite(V_OUT2, X_IN2 / 4);

  static  unsigned long freq = CENTRE;
  float X_IN;    //记录声音信号 
  X_IN = analogRead(V_IN);
  Serial.println(X_IN);
  dds.setClock(CALIBRATION);
  
  // Apply FM modulation
  set_modulated_frequency(X_IN);

  if ((freq += STEP) > CENTRE+DEVIATION)
    freq = CENTRE-DEVIATION;    // Start again at the bottom of the sweep
  next_update += RATE;
  unsigned long wait = next_update-millis();
  if (wait < 1000)            // Wait until RATE ms have elapsed since the last update
    delay(wait);                // Unless the update took longer than RATE
  else
    next_update = millis()+RATE;  // Resynchronise if possible, things exploded
}
