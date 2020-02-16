# Arduino IRIG Timecode library
IRIG Timecode encoding/decoding library for Arduino and STM32duino

Support for both IRIG A and IRIG B, however the library does not support IRIG with year nor SBS in either IRIG A or IRIG B mode. 

***Warning*** Some embedded processors could have trouble with IRIG A due to its 100 microsecond precision requirement, a >50MHz processor is recommended, however I have been able to rather easily generate valid IRIG A signals on an Arduino Uno, but when they are not always properly received 

# Documentation
## IRIG Timecode structure
```cpp
struct irig_time_t {
	uint8_t secs, mins, hours;
	uint16_t day_of_year;
	uint8_t tenths_of_secs;
	
	void fixup();
	void uptime();
	uint16_t to_strn(char* str, uint16_t n);
};
```
#### Fields
- `secs`: Seconds, from 0 to 60
- `mins`: Minutes, from 0 to 60
- `hours`: Hours, from 0 to 24
- `day_of_year`: Day of year, from 0 to 365
- `tenths_of_secs`: 1/10 Seconds, from 0 to 10
#### Methods
- `fixup()`: Called by `IRIG_TX::send`, makes sure time fields are in range
- `uptime()`: Copies current system uptime into struct
- `to_strn(char* str, uint16_t n)`: Copies contents into `str`, up to `n`, returns length

# IRIG Modes
0. `IRIG_A`
1. `IRIG_B`

If an invalid mode is specified, it will default to `IRIG_B`

# IRIG_TX
- `IRIG_TX(uint8_t mode)`: Constructor for the `IRIG_TX` class, takes a mode listed above
- `begin(int16_t pin)`: Initialize output on `pin`
- `send(irig_time_t timecode)`: Sends the timecode
- `uint16_t getSendRate()`: Gets the recommended interval in which to send timecodes

# IRIG_RX
- `IRIG_RX(uint8_t mode)`: Constructor for the `IRIG_RX` class, takes a mode listed above
- `begin(int16_t pin)`: Initialize input on `pin`, iterrupts not yet supported
- `uint8_t recv(irig_time_t* into)`: Receives timecode into `into`, returns 1 on success, blocks until a specified timeout has been reached (frame_time * 3 * 10)
