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
 * This example gets the time via NTP, using an ethernet shield and DHCP
 * Then transmits the time via IRIG, and corrects for the delay between IRIG
 * transmissions and NTP receive (in case of network packet loss)
 * 
 * ======================================================================================
 */

#include "SPI.h"
#include "Ethernet.h"
#include "EthernetUdp.h"
#include "IRIG.h"

#define IRIG_MODE     IRIG_B           // The IRIG mode to transmit
#define IRIG_PIN      8                // The pin to transmit IRIG on
#define ETHER_CS      10               // The ethernet chip's chip select
#define NTP_URL       "time.nist.gov"  // The NTP server address
#define NTP_RATE      300              // The rate to poll the NTP server
#define HOLDOVER_TIME 3600000          // The amount of time to work without an NTP packet, default 1 hour
#define MAINTAIN_RATE 60000            // The rate at which the DHCP lease should me maintained, default 1 min
#define SEND_RATE     15000            // The rate at which IRIG packets should be sent, default 15 seconds

uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
unsigned uint16_t localPort = 8888;
#define NTP_PACKET_SIZE 48
#define SEVENTY_YEARS 2208988800UL
uint8_t udp_buf[NTP_PACKET_SIZE];

IRIG_TX tx(IRIG_MODE);
EthernetUDP udp;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing ethernet!");
  Ethernet.init(ETHER_CS);
  if(Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet via DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("No ethernet shield");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet not connected");
    }
    while(1);
  }
  Serial.print("Initialized ethernet with IP: ");
  Serial.println(Ethernet.localIP());
  tx.begin(IRIG_PIN);
  udp.begin(localPort);
}

uint64_t last_ntp_tx = 0;
uint64_t last_correct_t = 0;
uint64_t last_irig_time = 0;
uint64_t last_maintain_time = 0;
uint64_t holdover_end_t = 0;
uint64_t txp = 0, rxp = 0;
char buf[48];
irig_time_t utc;
void loop() {
  if(millis() - last_irig_time > SEND_RATE && millis() < holdover_end_t) {
    irig_time_t utc_off;
    memcpy(&utc_off, &utc, sizeof(irig_time_t));
    last_irig_time = millis();
    uint32_t h = millis() - last_correct_t;
    utc_off.add_ms(h);
    tx.send(utc_off);
    
    Serial.print("IRIG Out: ");
    utc_off.to_strn(buf, 48);
    Serial.print(buf);
    Serial.print(", holdover: ");
    Serial.println(h);
  }
  if(millis() - last_ntp_tx > NTP_RATE) {
    sendNTPpacket(NTP_URL, 0);
    txp++; last_ntp_tx = millis();
  }
  uint16_t size = udp.parsePacket();
  if(size > 0 && udp.available() >= NTP_PACKET_SIZE) {
    udp.read(udp_buf, NTP_PACKET_SIZE);
    uint32_t h = udp_buf[40]; h <<= 8; h += udp_buf[41];
    uint32_t l = udp_buf[42]; l <<= 8; l += udp_buf[43];
    uint32_t epoch = (h << 16 | l) - SEVENTY_YEARS;
    utc.tenths_of_secs = 0;
    utc.hours = (epoch % 86400L) / 3600;
    utc.mins  = (epoch % 3600) / 60;
    utc.secs  = epoch % 60;
    utc.day_of_year = (epoch % 31556983) / 86400 + 1;
    
    Serial.print("Got NTP: ");
    utc.to_strn(buf, 48);
    Serial.println(buf);
    last_correct_t = millis();
    holdover_end_t = millis() + HOLDOVER_TIME;
  }
  if(millis() - last_maintain_time > MAINTAIN_RATE) {
    last_maintain_time = millis();
    maintain();
  }
}

void maintain() {
  uint8_t v = Ethernet.maintain(), pa = 0;
  switch(v) {
    case 1: Serial.println("Ethernet renewal failed!"); break;
    case 2: Serial.println("Ethernet renewal suceeded!"); pa = 1; break;
    case 3: Serial.println("Ethernet rebind failed!"); break;
    case 4: Serial.println("Ethernet rebind suceeded!"); pa = 1; break;
  }
  if(pa) {
    Serial.print("IP Address: ");
    Serial.println(Ethernet.localIP());
  }
}
uint8_t sendNTPpacket(char* address, uint8_t stratum) {
  memset(udp_buf, 0, NTP_PACKET_SIZE);
  
  udp_buf[0] = 0b11100011;   // LI, Version, Mode
  udp_buf[1] = stratum;      // Stratum, or type of clock
  udp_buf[2] = 6;            // Polling Interval
  udp_buf[3] = 0xEC;         // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  udp_buf[12]  = 49; udp_buf[13]  = 0x4E;
  udp_buf[14]  = 49; udp_buf[15]  = 52;
  
  udp.beginPacket(address, 123);
  udp.write(udp_buf, NTP_PACKET_SIZE);
  return udp.endPacket();
}
