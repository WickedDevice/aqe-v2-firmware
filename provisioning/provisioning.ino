/*
*  Production setup for AQE.
*  After egg has been preregistered, its api key and feed ID are provisioned from Xively
*  User can then enter calibration data for sensors.
*/
#include <sha1.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <ccspi.h>
#include <SPI.h>
#include <WildFire.h>
#include <WildFire_CC3000.h>
#include "MemoryLocations.h"

WildFire wf;
WildFire_CC3000 cc3000;
int sm_button = 5;
int time = 0;
uint8_t mymac[6];// = {10,10,10,10,10,10}; //for testing           
#define WEBSITE "api.xively.com"

#define PROVISIONING_STATUS_GOOD 0x73
boolean activated = false;

#define soft_reset()        \
do                          \
{                           \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
} while(0)

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();
    return;
}

double CO_M;
double NO2_M;
double O3_M;
const long magic_number = 0x5485;
char buffer[700];
uint32_t ip;

#define WLAN_SSID       "NETWORK_ID"           // cannot be longer than 32 characters!
#define WLAN_PASS       "PASSWORD"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

void setup() {
  wf.begin();
  pinMode(sm_button, INPUT_PULLUP);
  Serial.begin(115200);

  Serial.println("Welcome to AQE calibration setup!");
  Serial.println("For best experience, please use the Arduino IDE.");
  Serial.println();
  
  //manual connection -- no smartconfig for production streamlining
   /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) 
  {
    delay(100); // ToDo: Insert a DHCP timeout!
    time += 100;
    wdt_reset();
    if (time > 20000) {
      time = 0;
      Serial.println(F("DHCP failed!"));\
      delay(2000);
      soft_reset();
    }
    }
  
  while(!displayConnectionDetails());
  
    // Get the website IP & print it
  ip = 0;
  while(ip == 0) {
    cc3000.getHostByName("api.xively.com", &ip);
  }
  cc3000.printIPdotsRev(ip);
  Serial.println(); 
   
  activateWithCosm();
  
  initialize_eeprom();
  Serial.println();
  Serial.println("AQE setup complete.");
}

void loop() {
  //nothing to do here...   
}

/*
*  Checks EEPROM to see if egg already has calibration data
*/
void initialize_eeprom() {
   long magic;
  eeprom_read_block(&magic, (const void*)CAL_MAGIC, sizeof(float));
  if(magic != magic_number) {
    Serial.println("\nSetting sensor calibration values....");
    _initialize_eeprom();
  }
  else {
    Serial.println("It looks like this egg has already been programmed with calibration data!");
    Serial.println();    
    eeprom_read_block(&CO_M, (const void*)CO_cal, sizeof(float));
    eeprom_read_block(&NO2_M, (const void*)NO2_cal, sizeof(float));
    eeprom_read_block(&O3_M, (const void*)O3_cal, sizeof(float));
    Serial.println("The current values are :");
    Serial.print("CO sensor : "); printDouble(CO_M, 6); Serial.println();
    Serial.print("NO2 sensor : "); printDouble(NO2_M, 6); Serial.println();
    Serial.print("O3 sensor : "); printDouble(O3_M, 6); Serial.println();
    Serial.println();
    Serial.println("Would you like to reset these values? Current data will be overwritten! (y/n)");
    while(Serial.available() <= 0);
    if (Serial.available() > 0) {
      char response = Serial.read();
      if (response == 'y') {
        _initialize_eeprom();
      }
    }   
  }
  Serial.println();
}
int done = 0;
/*
*  Takes user input from serial monitor and writes those values to EEPROM
*/
void _initialize_eeprom() {
  while (!done) {
    Serial.println();
    Serial.println("IMPORTANT: Please include the leading 0 before the decimal point.");
    Serial.println();
    Serial.println("Enter calibration value for the CO sensor: ");
    while(Serial.available() <= 0);
    if (Serial.available() > 0) {
      CO_M = Serial.parseFloat();
    }
    Serial.println("Enter calibration value for the NO2 sensor: ");
      while(Serial.available() <= 0);
    if (Serial.available() > 0) {
      NO2_M = Serial.parseFloat();
    }
      Serial.println("Enter calibration value for the O3 sensor: ");
      while(Serial.available() <= 0);
    if (Serial.available() > 0) {
      O3_M = Serial.parseFloat();
    }
    Serial.print("CO sensor : ");
    printDouble(CO_M, 6);
    Serial.println();
    Serial.print("NO2 sensor : ");
    printDouble(NO2_M, 6);
    Serial.println();
    Serial.print("O3 sensor : ");
    printDouble(O3_M, 6);
    Serial.println();
    Serial.println("Are these values correct? (y/n)");
    while(Serial.available() <= 0);
    char response;
    if (Serial.available() > 0) {
      response = Serial.read();
    }
    if (response == 'y') {
      eeprom_write_block(&CO_M, (void*)CO_cal, sizeof(float));
      eeprom_write_block(&NO2_M, (void*)NO2_cal, sizeof(float));
      eeprom_write_block(&O3_M, (void*)O3_cal, sizeof(float));
      eeprom_write_block(&magic_number, (void*)CAL_MAGIC, sizeof(long));
      eeprom_read_block(&CO_M, (const void*)CO_cal, sizeof(float));
      eeprom_read_block(&NO2_M, (const void*)NO2_cal, sizeof(float));
      eeprom_read_block(&O3_M, (const void*)O3_cal, sizeof(float));
      Serial.println();
      Serial.println("Wrote calibration data :");
      Serial.print("CO sensor : ");
      printDouble(CO_M, 6);
      Serial.println();
      Serial.print("NO2 sensor : ");
      printDouble(NO2_M, 6);
      Serial.println();
      Serial.print("O3 sensor : ");
      printDouble(O3_M, 6);
      Serial.println();
      Serial.println();
      done = 1;
    }
    else {
      done = 0;
    }
  }
}

bool displayConnectionDetails(void) {
  uint32_t addr, netmask, gateway, dhcpserv, dnsserv;

  if(!cc3000.getIPAddress(&addr, &netmask, &gateway, &dhcpserv, &dnsserv))
    return false;

  Serial.print(F("IP Addr: ")); cc3000.printIPdotsRev(addr);
  Serial.print(F("\r\nNetmask: ")); cc3000.printIPdotsRev(netmask);
  Serial.print(F("\r\nGateway: ")); cc3000.printIPdotsRev(gateway);
  Serial.print(F("\r\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
  Serial.print(F("\r\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
  Serial.println();
  return true;
}

//--- for generating the SHA1 Hash
static void convertByteArrayToAsciiHex(uint8_t* hash, char * returnString, uint8_t byte_array_length) {
  const char digit2Ascii[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  for (int i=0; i<byte_array_length; i++) {
    returnString[2*i    ] = digit2Ascii[hash[i]>>4];
    returnString[2*i + 1] = digit2Ascii[hash[i]&0xf];
  }
  returnString[2*(byte_array_length-1) + 2] = '\0';
}

// computes the activation_url
void computeActivationUrl(char * activation_url){
  
  //Air Quality Egg v01 Product Secret:
  //e54c89cf2b916934a288304b6f7630e5b9aad8c8
  //TODO: update to AQE v2 product secret
  uint8_t deviceKey[] ={ 
    0xe5,0x4c,0x89,0xcf,0x2b,0x91,0x69,0x34,0xa2,0x88,
    0x30,0x4b,0x6f,0x76,0x30,0xe5,0xb9,0xaa,0xd8,0xc8
  };
  
  //--- pull MAC address from WildFire, stick it in serialNumber
 
  if (!cc3000.getMacAddress(mymac)) {
  	Serial.println("Could not get MAC address!");
  	while (1); //TODO: is this correct behavior?
   }
 
  char serial_number[18] = {0};
  Serial.println();
  Serial.println(F("**********************************************************************************"));
  Serial.print(F("Serial #: "));
  for(uint8_t ii = 0; ii < 6; ii++){
    convertByteArrayToAsciiHex(mymac + ii, serial_number + 3*ii, 1);
    if(ii == 5) serial_number[3*ii+2] = '\0';
    else serial_number[3*ii+2] = ':';
  }
  Serial.println(serial_number);  
  Serial.println(F("**********************************************************************************"));
  Serial.println();
  //--- generate SHA1
  Serial.println(F("compute sha: "));
  
  #define HMAC_LENGTH 20
  Sha1.initHmac(deviceKey,HMAC_LENGTH); 
  Sha1.print(serial_number);
  
  convertByteArrayToAsciiHex(Sha1.resultHmac(), activation_url, HMAC_LENGTH); 
  
  strcat_P(activation_url, PSTR("/activate\0"));
  Serial.println(activation_url);   
}

/*
*  Tokenizes the buffer (contents of webpage) to find feed id and api key, then writes to EEPROM
*/
static void provisioningCallback (char *dbuf) {
  boolean foundFeedId = false;
  boolean foundApiKey = false;
  uint8_t api_key_strlen = 0;
  uint8_t feed_id_strlen = 0;
  
  Serial.println(F(">>> PROVISIONING CALLBACK"));  
  
  char* token = strtok((char *)dbuf, ":,\"\n"); //tokenize

  while ( token != 0 ){

    Serial.print(F("Token: "));
    Serial.println(token);

    //--- setting the apikey and feedid (after they've been found)
    if (foundApiKey){
      // put the token in EEPROM to be retrieved later
      setApiKeyInEEPROM(token);
      api_key_strlen = strlen(token);
      foundApiKey = false;
    } 
    if (foundFeedId){ 
      // put the token in EEPROM to be retrieved later
      setFeedIdInEEPROM(token);
      feed_id_strlen = strlen(token);
      foundFeedId = false;
    }

    //--- looking for the apikey and feedid
    if(strncmp_P(token, PSTR("apikey"),6)==0){
      Serial.print(F(">>>FOUND APIKEY\n"));
      foundApiKey = true; //the next token will be the apikey
    } 
    if(strncmp_P(token,PSTR("feed_id"),6)==0){
      Serial.print(F(">>>FOUND FEED_ID\n"));
      foundFeedId = true; //the next token will be the feedid
    }

    if(feed_id_strlen > 0 && api_key_strlen > 0){
      activated = true;
      Serial.print(F("API KEY LENGTH = "));
      Serial.println(api_key_strlen);        
      Serial.print(F("FEED LENGTH = "));
      Serial.println(feed_id_strlen);
      eeprom_write_byte((uint8_t *) ACTIVATION_STATUS_EEPROM_ADDRESS, PROVISIONING_STATUS_GOOD);   
      delay(2000);
      return; 
    }
    
    token = strtok (0, ":,\"\n"); // advance token pointer
  }

  Serial.println(F("..."));
  Serial.println(F("Could not get API key and feed ID. Has this egg been preregistered correctly?"));
  while(1);
}

#define MAX_ACTIVATION_ATTEMPTS         4   // about 1 minutes worth of attempts
#define INITIAL_PACKET_DELAY_MS      10000L // 10 seconds
#define ACTIVATION_RETRY_INTERVAL_MS 15000L // retry every 15 seconds
void activateWithCosm(){
  uint8_t test = eeprom_read_byte((const uint8_t *) ACTIVATION_STATUS_EEPROM_ADDRESS);
  if(test == PROVISIONING_STATUS_GOOD){
    Serial.println(F("Previously provisioned"));
    char api_key[API_KEY_LENGTH];
    char feedID[FEED_ID_LENGTH];
    eeprom_read_block(api_key, (const void*)API_KEY_EEPROM_ADDRESS, API_KEY_LENGTH);
    eeprom_read_block(feedID, (const void*)FEED_ID_EEPROM_ADDRESS, FEED_ID_LENGTH); 
    Serial.print("API key: "); Serial.println(api_key);
    Serial.print("Feed ID: "); Serial.println(feedID);   
    return;   
  }
  else{ 
    Serial.print(F("Uninitialized - test value = "));
    Serial.println(test, HEX);
    doProvisioning();
  }
}

/*
*  Computes activation code for this egg, then connects to api.xively.com and requests api key and feedID for this activation code
*/
void doProvisioning(){
  uint32_t timer = INITIAL_PACKET_DELAY_MS; // initial waiting period
  uint32_t current_time = 0;
  uint16_t numAttempts = 0;  
  char activation_url[ACTIVATION_URL_LENGTH];  
 
  computeActivationUrl(activation_url);
 
  activated = false;
  for(;;){        
    if(numAttempts == MAX_ACTIVATION_ATTEMPTS){
      Serial.print(F("Provisioning failed "));
      Serial.print(MAX_ACTIVATION_ATTEMPTS);
      Serial.println(F(" times, restarting"));
      Serial.flush();
      delay(10000);      
      soft_reset(); // better reset at this point...
    }
    
    WildFire_CC3000_Client client = cc3000.connectTCP(ip, 80);
    if (client.connected()) {
      client.fastrprint(F("GET "));
      client.fastrprint("/v2/devices/");
      client.fastrprint(activation_url);
      client.fastrprint(F(" HTTP/1.0\r\n"));
      client.fastrprint(F("Host: ")); client.fastrprint("api.xively.com"); client.fastrprint(F("\r\n"));
      client.fastrprint(F("\r\n"));
      client.println();
    } else {
      Serial.println(F("Connection failed"));    
      return;
    }
    
    char dbuf[700] = "";
    current_time = millis();
    if (current_time > timer) {
      timer = current_time + ACTIVATION_RETRY_INTERVAL_MS;
      Serial.println();
      if(!activated){
        Serial.print(F("<<< REQ "));
      unsigned long lastRead = millis();
      while (client.connected() && (millis() - lastRead < 150000)) {
        int i = 0;
        while (client.available()) {
          char c = client.read();
          dbuf[i] = c;
          i++;
          lastRead = millis();
        }
      }
      client.close();
        strcat(dbuf, '\0');
        Serial.println(dbuf);
        provisioningCallback(dbuf);
        if (activated) {
          return;
        }
        numAttempts++;
        //URL hit is something like: 
        //http://api.cosm.com/v2/devices/0aad07fc55fb297074a48d0cb9ca950e0d92ed72/activate
      }  
      else {
        Serial.println(F(">>ACTIVATED<<"));
        // the callback function will have stored the api key and feed id in EEPROM
        return; // we are activated
      }
    }     
  }   
}

void setApiKeyInEEPROM(char * api_key){
  eeprom_write_block(api_key, (void*)API_KEY_EEPROM_ADDRESS, API_KEY_LENGTH);
  Serial.print(F("Wrote API KEY to EEPROM: "));
  Serial.println(api_key);
}

void setFeedIdInEEPROM(char * feed_id){
  eeprom_write_block(feed_id, (void*)FEED_ID_EEPROM_ADDRESS, FEED_ID_LENGTH);
  Serial.print(F("Wrote FEED ID to EEPROM: "));
  Serial.println(feed_id);
}

void printDouble( double val, byte precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)
if (val < 0 && val > -1) {
  Serial.print("-");
}
Serial.print (int(val));  //prints the int part
  if( precision > 0) {
    Serial.print("."); // print the decimal point
    unsigned long frac;
    unsigned long mult = 1;
    byte padding = precision -1;
    while(precision--)
       mult *=10;
       
    if(val >= 0)
      frac = (val - int(val)) * mult;
    else
      frac = (int(val)- val ) * mult;
    unsigned long frac1 = frac;
    while( frac1 /= 10 )
      padding--;
    while(  padding--)
      Serial.print("0");
    Serial.print(frac,DEC) ;
  }
} 
