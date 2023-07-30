#include "Adafruit_EEPROM_I2C.h"
#include "Adafruit_FRAM_I2C.h"

Adafruit_EEPROM_I2C i2ceeprom;

#define EEPROM_ADDR 0x50  // the default address!

#define KEY_SIZE 16


struct cipherVector {
  char name[16];       //name of id, password
  char id[32];         //contains userid or email
  byte cipher[KEY_SIZE];  //encrypted password
};

// cipherVector cipher1 = {
//   "Gmail",
//   "posterskensri@gmail.com",
//   { 0x5D, 0x16, 0x26, 0x6E, 0x10, 0xB8, 0xE5, 0xFE,
//     0x76, 0x4C, 0x3A, 0x2B, 0xD6, 0x80, 0xAE, 0xF6 }  //123456789012345
// };

byte buffer[sizeof(cipherVector)];
cipherVector f;

void setup(void) {
  uint16_t num;

  Serial.begin(115200);
  delay(2000);
  Serial.print("lol");

  Wire.setSDA(0);
  Wire.setSCL(1);

  if (i2ceeprom.begin(0x50, &Wire)) {  // you can stick the new i2c addr in here, e.g. begin(0x51);
    Serial.println("Found I2C EEPROM");
  } else {
    Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1) delay(10);
  }

  // memcpy(buffer, (void*)&cipher1, sizeof(cipher1)); //to write
  // i2ceeprom.write(0x00, buffer, sizeof(buffer));

  i2ceeprom.read(0x00, buffer, sizeof(cipherVector));//to read
  memcpy((cipherVector*)&f, buffer, sizeof(cipherVector));
  
  Serial.print("Read back float value: ");
  Serial.println(f.id);
  Serial.println();
}

void loop(void) {
}
