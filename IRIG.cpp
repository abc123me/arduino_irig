#include "IRIG.h"

// #define __DEBUG_IRIG

#ifdef __DEBUG_IRIG
	#define _DBG_PRINT(s) Serial.print(s);
	#define _DBG_PRINTLN(s) Serial.println(s);
#else
	#define _DBG_PRINT(s)
	#define _DBG_PRINTLN(s)
#endif

/* __  ______________    ____________  __
  / / / /_  __/  _/ /   /  _/_  __/\ \/ /
 / / / / / /  / // /    / /  / /    \  / 
/ /_/ / / / _/ // /____/ /  / /     / /  
\____/ /_/ /___/_____/___/ /_/     /_/  */
void to_bcd2(uint8_t dat, uint8_t* ones, uint8_t* tens) {
	uint8_t t = 0;
	while(dat > 9) { t++; dat -= 10; }
	*tens = t; *ones = dat;
}
void to_bcd3(uint16_t dat, uint8_t* ones, uint8_t* tens, uint8_t* hdrs) {
	uint8_t t = 0, h = 0;
	while(dat > 99) { h++; dat -= 100; }
	while(dat > 9) { t++; dat -= 10; }
	*hdrs = h; *tens = t; *ones = dat;
}

/*  ________  __________   ____  _______________________    ________
   /  _/ __ \/  _/ ____/  / __ \/ ____/ ____/ ____/  _/ |  / / ____/
   / // /_/ // // / __   / /_/ / __/ / /   / __/  / / | | / / __/   
 _/ // _, _// // /_/ /  / _, _/ /___/ /___/ /____/ /  | |/ / /___   
/___/_/ |_/___/\____/  /_/ |_/_____/\____/_____/___/  |___/_____/  */
IRIG_RX::IRIG_RX(uint8_t mode) {
	uint16_t time0, time1, timeI, timeE;
	switch(mode) {
		case IRIG_A:
			time0 = 200; time1 = 500;
			timeI = 800; timeE = 150;
			timeout = 3000;
		break;
		default:
			time0 = 2000; time1 = 5000;
			timeI = 8000; timeE = 1500;
			timeout = 30000;
		break;
	}
	time1A = time1 - timeE; time1B = time1 + timeE;
	time0A = time0 - timeE; time0B = time0 + timeE;
	timeIA = timeI - timeE; timeIB = timeI + timeE;
}
void IRIG_RX::begin(int16_t pin) {
	this->pin = pin;
	pinMode(pin, INPUT);
}
uint8_t from_bcd(uint8_t bcd) {
	return (bcd & 0xF) + 10 * (bcd >> 4);
}
void IRIG_RX::process_buf(irig_time_t* into) {
	into->secs = from_bcd(recv_buf[0]);
	into->mins = from_bcd(recv_buf[1]);
	into->hours = from_bcd(recv_buf[2]);
	into->day_of_year = from_bcd(recv_buf[3]) + 100 * (recv_buf[4] & 0b11);
	into->tenths_of_secs = recv_buf[4] >> 4;
}
uint8_t IRIG_RX::recv(irig_time_t* into) { return recv(into, timeout * 10); }
uint8_t IRIG_RX::recv(irig_time_t* into, uint32_t timeout_us) {
	if(micros() - last_pulse < timeout)
		return 0;
	unsigned long lt;
	lt = pulseIn(pin, HIGH, timeout_us);
	if(lt < timeIA || lt > timeIB)
		return 0;
	lt = pulseIn(pin, HIGH, timeout);
	if(lt > timeIA && lt < timeIB) {
		_DBG_PRINTLN("recv(): init pulse");
		uint8_t bit5 = 1, pulse = 0, dat = 0, i = 1;
		recv_buf_pos = 0;
		while(1) {
			lt = pulseIn(pin, HIGH, timeout);
			if(lt > time0A && lt < time0B)
				pulse = 0;
			else if(lt > time1A && lt < time1B)
				pulse = 1;
			else if(lt > timeIA && lt < timeIB)
				pulse = 2;
			else if(lt == 0 || recv_buf_pos >= 6) {
				process_buf(into);
				return 1;
			}
			if(pulse == 2){
				_DBG_PRINT("recv_buf[");
				_DBG_PRINT(recv_buf_pos);
				_DBG_PRINT("]: ");
				_DBG_PRINTLN(dat);
				recv_buf[recv_buf_pos++] = dat;
				dat = 0; i = 1; bit5 = 1;
			} else if(i == 0b10000 && bit5) { // Fuck the 5th bit, that guy always ruins the party
				bit5 = 0;
			} else {
				if(pulse) dat |= i;
				else      dat &= ~i;
				i <<= 1;
			}
		}
		last_pulse = micros();
	}
	return 0;
}

/*    ________  __________   __________  ___    _   _______ __  _____________
   /  _/ __ \/  _/ ____/  /_  __/ __ \/   |  / | / / ___//  |/  /  _/_  __/
   / // /_/ // // / __     / / / /_/ / /| | /  |/ /\__ \/ /|_/ // /  / /   
 _/ // _, _// // /_/ /    / / / _, _/ ___ |/ /|  /___/ / /  / // /  / /    
/___/_/ |_/___/\____/    /_/ /_/ |_/_/  |_/_/ |_//____/_/  /_/___/ /_/   */
IRIG_TX::IRIG_TX(uint8_t mode) {
	uint16_t mul = 1000;
	switch(mode) {
		case IRIG_A: mul = 100; break;
	}
	time0A = 2 * mul; time0B = 8 * mul;
	time1A = 5 * mul; time1B = 5 * mul;
	timeIA = 8 * mul; timeIB = 2 * mul;
	ftimeMS = 1 * mul; 
}
uint16_t IRIG_TX::getSendRate() {
	return ftimeMS;
}
void IRIG_TX::begin(int16_t pin) {
	this->pin = pin;
	pinMode(pin, OUTPUT);
}
//TODO: Improve speed of function, ie. calculate send buffer, and send it
void IRIG_TX::send(irig_time_t tc) {
	tc.fixup();
	_DBG_PRINT("Sending: ");

	// START FRAME
	send_pos_ind();
	send_pos_ind();
	
	// SECONDS FRAME
	to_bcd2(tc.secs, &bcd1, &bcd10);  
	send_nibble(bcd1);
	send_nibble(bcd10 << 1);
	send_pos_ind();
	
	// MINUTES FRAME
	send_bcd9(tc.mins);
	
	// HOURS FRAME
	send_bcd9(tc.hours);
	
	// DOY FRAME
	to_bcd3(tc.day_of_year, &bcd1, &bcd10, &bcd100);
	send_nibble(bcd1);
	send_bit(0);
	send_nibble(bcd10);
	send_pos_ind();
	send_nibble(bcd100);
	send_bit(0);

	// TOS FRAME
	send_nibble(tc.tenths_of_secs);
	send_pos_ind();
	
	_DBG_PRINTLN();
}
void IRIG_TX::send_bcd9(uint8_t num) {
	to_bcd2(num, &bcd1, &bcd10);
	send_nibble(bcd1);
	send_bit(0);
	send_nibble(bcd10);
	send_pos_ind();
}
void IRIG_TX::send_nibble(uint8_t nibb) {
	send_bit(nibb & 1);
	send_bit(nibb & 2);
	send_bit(nibb & 4);
	send_bit(nibb & 8);
}
inline void IRIG_TX::send_pos_ind() {
	_DBG_PRINT('P');
	pulse(timeIA, timeIB);
}
inline void IRIG_TX::send_bit(uint8_t bit) {
	_DBG_PRINT(bit ? '1' : '0');
	if(bit) pulse(time1A, time1B);
	else pulse(time0A, time0B);
}
inline void IRIG_TX::pulse(uint16_t a, uint16_t b) {
	digitalWrite(pin, 1);
	delayMicroseconds(a);
	digitalWrite(pin, 0);
	delayMicroseconds(b);
}

/*  ________  __________   ____________  ___________________  ____  ______
   /  _/ __ \/  _/ ____/  /_  __/  _/  |/  / ____/ ____/ __ \/ __ \/ ____/
   / // /_/ // // / __     / /  / // /|_/ / __/ / /   / / / / / / / __/   
 _/ // _, _// // /_/ /    / / _/ // /  / / /___/ /___/ /_/ / /_/ / /___   
/___/_/ |_/___/\____/    /_/ /___/_/  /_/_____/\____/\____/_____/_____/   */
void irig_time_t::fixup() {
	while(tenths_of_secs > 9) { tenths_of_secs -= 10; secs++; }
	while(secs > 59) { secs -= 60; mins++; }
	while(mins > 59) { mins -= 60; hours++; }
	while(hours > 23) { hours -= 24; day_of_year++; }
	while(day_of_year > 364) { day_of_year -= 365; }
}
uint16_t irig_time_t::to_strn(char* str, uint16_t n) {
	return snprintf(str, n, "%d:%d:%d.%d day %d", hours, mins, secs, tenths_of_secs, day_of_year);
}
void irig_time_t::uptime() {
	uint64_t ms = millis();
	day_of_year = ms / MS_PER_DAY;
	ms -= day_of_year * MS_PER_DAY;
	hours = ms / MS_PER_HOUR;
	ms -= hours * MS_PER_HOUR;
	mins = ms / MS_PER_MINUTE;
	ms -= mins * MS_PER_MINUTE;
	secs = ms / 1000;
	ms -= secs * 1000;
	tenths_of_secs = ms / 100;
}
void irig_time_t::add_ms(uint32_t tdiff_ms) {
	uint8_t amo = tdiff_ms % 100;
	tenths_of_secs += amo / 10; tdiff_ms -= amo;
	add_s(tdiff_ms / 1000);
}
void irig_time_t::add_s(uint32_t tdiff) {
	secs += tdiff % 60;
	add_m(tdiff / 60);
}
void irig_time_t::add_m(uint16_t tdiff) {
	mins += tdiff % 60;
	add_h(tdiff / 60);
}
void irig_time_t::add_h(uint16_t tdiff) {
	hours += tdiff % 24;
	add_d(tdiff / 24);
}
void irig_time_t::add_d(uint8_t tdiff) {
	day_of_year += tdiff;
	fixup();
}
