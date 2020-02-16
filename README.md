# Arduino IRIG Timecode library
IRIG Timecode encoding/decoding library for Arduino and STM32duino

## Documentation
### IRIG Timecode structure
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
- `to_strn(char* str, uint16_t n)`: Copies the struct contents into string `str`, up to length `n`, returns actual length copied
