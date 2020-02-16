/*
 *     ___             __      _                ________  __________
 *    /   |  _________/ /_  __(_)___  ____     /  _/ __ \/  _/ ____/
 *   / /| | / ___/ __  / / / / / __ \/ __ \    / // /_/ // // / __  
 *  / ___ |/ /  / /_/ / /_/ / / / / / /_/ /  _/ // _, _// // /_/ /  
 * /_/  |_/_/   \__,_/\__,_/_/_/ /_/\____/  /___/_/ |_/___/\____/
 * =====================================================================
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
 * This example sends the processor's uptime via an IRIG-B signal (without year or SBS)
 * on pin 13 of the microcontroller, once per seconds as specified by the IRIG standard
 * 
 * =====================================================================
 */

#include "IRIG.h"

IRIG_TX tx(IRIG_B);
//IRIG_TX tx(IRIG_A);

#define IRIG_PIN 13

void setup() {
  Serial.begin(9600);
  tx.begin(IRIG_PIN); 
}

char buf[64];
void loop() {
  static uint64_t last_t;
  if(millis() > last_t + tx.getSendRate()) {
    last_t = millis();
    irig_time_t tc;
    tc.uptime(); // Calculates and stores the system uptime into the timecode struct
    tx.send(tc);
    
    Serial.print("Sent frame: ");
    tc.to_strn(buf, 64);
    Serial.println(buf);
  }
}
