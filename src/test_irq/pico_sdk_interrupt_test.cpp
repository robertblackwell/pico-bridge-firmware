// #include <Arduino.h>
#include "hardware.gpio.h"
volatile int isr_M1A_count;
volatile int isr_M1B_count;
volatile int isr_M2A_count;
volatile int isr_M2B_count;
#define M1_PIN_E1 14
#define M1_PIN_M1 15
#define EC1_PIN_ENCODER_A 13
#define EC1_PIN_ENCODER_B 12

#define M2_PIN_E2 3
#define M2_PIN_M2 2
#define EC2_PIN_ENCODER_A 1
#define EC2_PIN_ENCODER_B 0



int value;
void isr_M1A_function() {
  isr_M1A_count += 1;
}
void isr_M1B_function() {
  isr_M1B_count += 2;
}
void isr_M2A_function() {
  isr_M2A_count += 3;
}
void isr_M2B_function() {
  isr_M2B_count += 4;
}
void setup() 
{
  value = 0;
  Serial.begin(115200);
  pinMode(M1_PIN_E1, OUTPUT);
  pinMode(M1_PIN_M1, OUTPUT);
  pinMode(EC1_PIN_ENCODER_A, INPUT);
  pinMode(EC1_PIN_ENCODER_B, INPUT);
  delay(4000);
  isr_M1A_count = 0;
  isr_M1B_count = 0;
  isr_M2A_count = 0;
  isr_M2B_count = 0;
  attachInterrupt(digitalPinToInterrupt(EC1_PIN_ENCODER_A), isr_M1A_function, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EC1_PIN_ENCODER_B), isr_M1B_function, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EC2_PIN_ENCODER_A), isr_M2A_function, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EC2_PIN_ENCODER_B), isr_M2B_function, CHANGE);
}
void loop() 
{
  value = (value == 0)? 1: 0; 
  delay(2000);
  digitalWrite(M1_PIN_M1, HIGH);
  analogWrite(M1_PIN_E1, 200);
  digitalWrite(M2_PIN_M2, HIGH);
  analogWrite(M2_PIN_E2, 200);

  Serial.println("Testing new boards");
  Serial.print("Hello isr_M1A_count: "); Serial.print(isr_M1A_count); 
  Serial.print("Hello isr_M1B_count: "); Serial.print(isr_M1B_count); 
  Serial.print("Hello isr_M2A_count: "); Serial.print(isr_M2A_count); 
  Serial.print("Hello isr_M2B_count: "); Serial.print(isr_M2B_count); 
  Serial.println(" ");

}



int main() {

    gpio_init(M1_PIN_M1);
    gpio_set_dir(M1_PIN_M1, GPIO_OUT);
    gpio_set_function(M1_PIN_E1, GPIO_FUNC_PWM);
    gpio_init(EC1_PIN_ENCODER_A);
    gpio_set_dir(EC1_PIN_ENCODER_B, GPIO_IN);

    gpio_init(M2_PIN_M2);
    gpio_set_dir(M2_PIN_M2, GPIO_OUT);
    gpio_set_function(M2_PIN_E2, GPIO_FUNC_PWM);
    gpio_init(EC2_PIN_ENCODER_A);
    gpio_set_dir(EC2_ENCODER_B, GPIO_IN);

    stdio_init_all();
    while(1) {
        gpio_put(LED_PIN, 0);
        sleep_ms(1000);
        gpio_put(LED_PIN,1);
        puts("Hello world \n");
        sleep_ms(1000);
}