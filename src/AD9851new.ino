#include <AD9851.h>
#include <SPI.h>

/* Sweep details */
#define CENTRE        48500000UL  /* Hz */ // 载波频率 48.5MHz
#define DEVIATION     25000      /* Deviation in Hz (above and below) */ // 偏移量 25KHz
#define STEP          0      /* Step in Hz. Set to 0 for constant frequency */
#define RATE          100       /* ms between steps */ // 两次更新之间的时间间隔
#define V_IN A0  //用于收集语音信号的引脚


#define CALIBRATION   5700        /* Calibration for my test module; it runs fast */

#define AD9851_FQ_UD_PIN      2
#define AD9851_RESET_PIN      3
// And MOSI=11, SCK=13

class MyAD9851 : public AD9851<AD9851_RESET_PIN, AD9851_FQ_UD_PIN> {};
MyAD9851 dds;
unsigned long next_update;        /* The millis() value for the next update */

void setup() {
  Serial.begin(38400);
  pinMode(V_IN,INPUT);
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
