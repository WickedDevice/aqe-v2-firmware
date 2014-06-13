#ifndef __AQE_MEMORY_LOCATIONS__
#define __AQE_MEMORY_LOCATIONS__

#define MCUSR_ADDRESS 0 //one byte to save value of MCUSR at startup
#define tinyWDT_STATUS	1 //one byte for status info
#define ACTIVATION_STATUS_EEPROM_ADDRESS 2  // one byte with a magic value
#define API_KEY_EEPROM_ADDRESS           3  // API KEY IS 48 characters + 1 for null terminator = 49 bytes
#define FEED_ID_EEPROM_ADDRESS           52 // FEED ID IS (at most) 9 characters + 1 for null terminator = 10 bytes
#define CAL_MAGIC	64 //4 bytes for long
#define CO_cal	68	//4 bytes for each float
#define NO2_cal	72
#define O3_cal	76




#define ACTIVATION_URL_LENGTH   52 // always 40 char code + "/activation" + null terminator
#define ACTIVATION_CODE_LENGTH  41 // always 40 characters + null terminator
#define API_KEY_LENGTH          49 // maximum length including null terminator
#define FEED_ID_LENGTH          17 // maximum length including null terminator

#endif