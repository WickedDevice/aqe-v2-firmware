/*
Use Arduino IDE for least rage
User will be prompted via serial monitor to enter calibration data for each sensor
*/

#include <WildFire.h>
#include <avr/eeprom.h>

#define eeprom_read_to(dst_p, eeprom_field, dst_size) eeprom_read_block(dst_p, (void *)offsetof(__eeprom_data, eeprom_field), MIN(dst_size, sizeof((__eeprom_data*)0)->eeprom_field))
#define eeprom_read(dst, eeprom_field) eeprom_read_to(&dst, eeprom_field, sizeof(dst))
#define eeprom_write_from(src_p, eeprom_field, src_size) eeprom_write_block(src_p, (void *)offsetof(__eeprom_data, eeprom_field), MIN(src_size, sizeof((__eeprom_data*)0)->eeprom_field))
#define eeprom_write(src, eeprom_field) { typeof(src) x = src; eeprom_write_from(&x, eeprom_field, sizeof(x)); }
#define MIN(x,y) ( x > y ? y : x )


WildFire wf(WILDFIRE_V2);
double CO_M;
double NO2_M;
double O3_M;
const long magic_number = 0x5406;

struct __eeprom_data {
  long magic;
  double CO_cal;
  double NO2_cal;
  double O3_cal;
};

void setup() {
 wf.begin();
 Serial.begin(115200);
 Serial.println("Welcome to AQE calibration setup!");
 Serial.println("For best experience, please use the Arduino IDE.");
 Serial.println(); 
}

void loop() {
  long magic;
  eeprom_read(magic, magic);
  if(magic != magic_number) {
    initialize_eeprom();
  }
  else {
    Serial.println("It looks like this egg has already been programmed with calibration data!");
    Serial.println();    
    eeprom_read(CO_M, CO_cal);
    eeprom_read(NO2_M, NO2_cal);
    eeprom_read(O3_M, O3_cal);
    Serial.println("The current values are :");
    Serial.print("CO sensor : "); printDouble(CO_M, 6); Serial.println();
    Serial.print("NO2 sensor : "); printDouble(NO2_M, 6); Serial.println();
    Serial.print("O3 sensor : "); printDouble(O3_M, 6); Serial.println();
    Serial.println();
    Serial.println("Would you like to reprogram? (y/n)");
    while(Serial.available() <= 0);
    if (Serial.available() > 0) {
      char response = Serial.read();
      if (response == 'y') {
        initialize_eeprom();
      }
    }   
  }
  
  while(1);
}

int done = 0;

void initialize_eeprom() {
  while (!done) {
    Serial.println();
    Serial.println("Please include the leading 0 before the decimal point.");
    Serial.println();
    Serial.println("Enter calibration value for the CO sensor: ");
    while(Serial.available() <= 0);
    if (Serial.available() > 0) {
      CO_M = Serial.parseFloat();
      //eeprom_write(Serial.parseFloat(), CO_cal);
    }
    Serial.println("Enter calibration value for the NO2 sensor: ");
      while(Serial.available() <= 0);
    if (Serial.available() > 0) {
      NO2_M = Serial.parseFloat();
      //eeprom_write(Serial.parseFloat(), NO2_cal);
    }
      Serial.println("Enter calibration value for the O3 sensor: ");
      while(Serial.available() <= 0);
    if (Serial.available() > 0) {
      O3_M = Serial.parseFloat();
      //eeprom_write(Serial.parseFloat(), O3_cal);
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
      eeprom_write(CO_M, CO_cal);
      eeprom_write(NO2_M, NO2_cal);
      eeprom_write(O3_M, O3_cal);
      eeprom_write(magic_number, magic);
      eeprom_read(CO_M, CO_cal);
      eeprom_read(NO2_M, NO2_cal);
      eeprom_read(O3_M, O3_cal);
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

void printDouble( double val, byte precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)
if (val < 0) {
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
