#ifndef _IRIG_H
#define _IRIG_H

#include "Arduino.h"

struct irig_time_t {
	uint8_t secs, mins, hours;
	uint16_t day_of_year;
	uint8_t tenths_of_secs;
	
	void fixup();
	void uptime();
	uint16_t to_strn(char* str, uint16_t n);
};

#define MS_PER_MINUTE	60000
#define MS_PER_HOUR		3600000
#define MS_PER_DAY		86400000

#define IRIG_FRAME_LEN 5

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
};
#endif
