#ifndef __AQE_MEMORY_LOCATIONS__
#define __AQE_MEMORY_LOCATIONS__

#define MCUSR_ADDRESS 0 //one byte to save value of MCUSR at startup
#define MAC_ADDRESS  1 //6 bytes
#define tinyWDT_STATUS	7 //one byte for status info
#define ACTIVATION_STATUS_EEPROM_ADDRESS 8  // one byte with a magic value
#define API_KEY_EEPROM_ADDRESS           9  // API KEY IS 48 characters + 1 for null terminator = 49 bytes
#define FEED_ID_EEPROM_ADDRESS           58 // FEED ID IS (at most) 9 characters + 1 for null terminator = 10 bytes
#define CAL_MAGIC	70 //4 bytes for long
#define CO_cal	74	//4 bytes for each float
#define NO2_cal	78
#define O3_cal	82




#define ACTIVATION_URL_LENGTH   52 // always 40 char code + "/activation" + null terminator
#define ACTIVATION_CODE_LENGTH  41 // always 40 characters + null terminator
#define API_KEY_LENGTH          49 // maximum length including null terminator
#define FEED_ID_LENGTH          17 // maximum length including null terminator

#endif