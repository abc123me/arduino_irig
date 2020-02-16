/*
 *     ___             __      _                ________  __________
 *    /   |  _________/ /_  __(_)___  ____     /  _/ __ \/  _/ ____/
 *   / /| | / ___/ __  / / / / / __ \/ __ \    / // /_/ // // / __  
 *  / ___ |/ /  / /_/ / /_/ / / / / / /_/ /  _/ // _, _// // /_/ /  
 * /_/  |_/_/   \__,_/\__,_/_/_/ /_/\____/  /___/_/ |_/___/\____/
 * ======================================================================================
 * 
 * Arduino/STM32 IRIG Timecode library (C) Jeremiah Lowe 2020
 * 
 * This software is under the GNU GPLv2 license, there is NO WARRANTY PROVIDED
 * 
 * Please redirect any issues or contributions to:
 * https://github.com/abc123me/arduino_irig
 * 
 * ======================================================================================
 * 
 * This example receives an IRIG signal and prints it to the serial port
 * The on-board LED is lit for 300ms if a signal is sucessfully received
 * 
 * ======================================================================================
 */

#include "IRIG.h"

#define IRIG_PIN 8
#define LED_PIN LED_BUILTIN

IRIG_RX rx(IRIG_B);
//IRIG_RX rx(IRIG_A);

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  rx.begin(IRIG_PIN);
}

char buf[64];
uint64_t led_t;
void loop() {
  irig_time_t tc;
  if(rx.recv(&tc)) {
    Serial.print("Got frame: ");
    tc.to_strn(buf, 64);
    Serial.println(buf);
    led_t = millis();
  }
  if(millis() - led_t < 300)
    digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);
}
