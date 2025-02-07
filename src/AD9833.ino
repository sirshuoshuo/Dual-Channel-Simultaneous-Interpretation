#include <MD_AD9833.h>
#include <SPI.h>

#define V_IN A0
#define V_OUT 9
MD_AD9833 AD(/*DATA*/5, /*CLK*/4, /*FSYNC*/3);

void setup(void)
{
  Serial.begin(115200); // 初始化串行通信，波特率为115200
  pinMode(V_IN, INPUT);
  pinMode(V_OUT, OUTPUT);
  
  AD.begin();
  AD.setMode(MD_AD9833::MODE_SINE);  // 选择正弦波模式
  AD.setFrequency(MD_AD9833::CHAN_0, 20000);  // 定义信号输出频率为10kHz
}

void loop() {
  int value_IN = analogRead(V_IN); // 读取模拟输入信号
  Serial.println(value_IN); // 打印输入信号到串行监视器
  
  // 将读取到的模拟信号值（0-1023）映射到PWM范围（0-255），并输出到V_OUT
  analogWrite(V_OUT, value_IN / 4);
}
