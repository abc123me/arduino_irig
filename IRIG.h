#ifndef _IRIG_H
#define _IRIG_H

#include "Arduino.h"

#ifdef __DEBUG_IRIG
	#define _DBG_PRINT(s) Serial.print(s);
	#define _DBG_PRINTLN(s) Serial.println(s);
#else
	#define _DBG_PRINT(s)
	#define _DBG_PRINTLN(s)
#endif

struct irig_time_t {
	uint8_t secs = 0;
	uint8_t mins = 0;
	uint8_t hours = 0;
	uint16_t day_of_year = 0;
	uint8_t tenths_of_secs = 0;
	
	void fixup();
	void uptime();
	void add_ms(uint32_t tdiff_ms);
	void add_s(uint32_t tdiff_s);
	void add_m(uint16_t tdiff_m);
	void add_h(uint16_t tdiff_h);
	void add_d(uint8_t tdiff_d);
	uint16_t to_strn(char* str, uint16_t n);
};

#define MS_PER_MINUTE	60000
#define MS_PER_HOUR		3600000
#define MS_PER_DAY		86400000

#define IRIG_FRAME_LEN 5
#define IRIG_MIN_FRAME_LEN 3

#define IRIG_A 0
#define IRIG_B 1

class IRIG_TX {
private:
	uint16_t time1A, time0A, timeIA;
	uint16_t time1B, time0B, timeIB;
	uint8_t bcd1, bcd10, bcd100;
	uint16_t ftimeMS = 0;
	int16_t pin;
  
	inline void pulse(uint16_t on, uint16_t off);
	inline void send_pos_ind();
	inline void send_bit(uint8_t bit);
	void send_nibble(uint8_t nibb);
	void send_bcd9(uint8_t num);
public:
	IRIG_TX(uint8_t mode);
	void begin(int16_t pin);
	void send(irig_time_t timecode);
	uint16_t getSendRate();
};
class IRIG_RX {
private:
	uint16_t time1A, time0A, timeIA;
	uint16_t time1B, time0B, timeIB;
	uint64_t last_pulse = 0;
	uint16_t timeout;
	uint8_t recv_buf[IRIG_FRAME_LEN];
	uint8_t recv_buf_pos = 0;
	int16_t pin;
	
	void process_buf(irig_time_t* into);
public:
	IRIG_RX(uint8_t mode);
	void begin(int16_t pin);
	uint8_t recv(irig_time_t* into);
	uint8_t recv(irig_time_t* into, uint32_t timeout_us);
};
#endif
