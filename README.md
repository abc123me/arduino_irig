# Arduino IRIG Timecode library
IRIG Timecode encoding/decoding library for Arduino and STM32duino

Support for both IRIG A and IRIG B, however the library does not support IRIG with year nor SBS in either IRIG A or IRIG B mode. 

***Warning*** Some embedded processors could have trouble with IRIG A due to its 100 microsecond precision requirement, a >20MHz processor is recommended, however I have been able to rather easily generate valid IRIG A signals on a 16MHz Arduino Uno, but the board struggles to receive them with a ~30% failure rate

# Documentation
## IRIG Timecode structure
```cpp
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
- `add_ms(uint32_t t) / add_s(uint32_t t)`: Adds a certain amount of time to the time struct

## IRIG Modes
- `#define IRIG_A 0`: 1ms IRIG, frames should be sent every 100ms
- `#define IRIG_B 1`: 10x slower version of IRIG, frames should be sent every 1 second, not accurate to the tenth of a second
- If an invalid mode is specified, it will default to `IRIG_B`

## IRIG_TX
- `IRIG_TX(uint8_t mode)`: Constructor for the `IRIG_TX` class, takes a mode listed above
- `begin(int16_t pin)`: Initialize output on `pin`
- `send(irig_time_t timecode)`: Sends the timecode
- `uint16_t getSendRate()`: Gets the recommended interval in which to send timecodes

## IRIG_RX
- `IRIG_RX(uint8_t mode)`: Constructor for the `IRIG_RX` class, takes a mode listed above
- `begin(int16_t pin)`: Initialize input on `pin`, iterrupts not yet supported
- `uint8_t recv(irig_time_t* into)`: Receives timecode into `into`, returns 1 on success, blocks until a specified timeout has been reached (frame_time * 3 * 10)
