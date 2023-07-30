#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include <string.h>
#include <Crypto.h>
#include <SHA256.h>
#include <AES.h>
#include <string.h>
#include "Adafruit_EEPROM_I2C.h"
#include "Adafruit_FRAM_I2C.h"

#define EEPROM_ADDR 0x50
#define KEY_SIZE 16

Adafruit_EEPROM_I2C i2ceeprom;

#define HASH_SIZE 32
#define KEY_SIZE 16

byte key_hash[HASH_SIZE];
byte decryptedText[KEY_SIZE];

SHA256 sha256;
AES256 aes256;

struct cipherVector {
  char name[16];          //name of id, password
  char id[32];            //contains userid or email
  byte cipher[KEY_SIZE];  //encrypted password
};

cipherVector const cipher1 = {
  //to test master password
  "Test",
  "123456789012345",
  { 0x5D, 0x16, 0x26, 0x6E, 0x10, 0xB8, 0xE5, 0xFE,
    0x76, 0x4C, 0x3A, 0x2B, 0xD6, 0x80, 0xAE, 0xF6 }  //123456789012345
};

byte buffer[sizeof(cipherVector)];
cipherVector f;

cipherVector data[64];
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define SCROLLSPEED 1

#define ROTARY_ENCODER_A_PIN 5
#define ROTARY_ENCODER_B_PIN 33
#define ROTARY_ENCODER_BUTTON_PIN 32
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 2
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

void copyArray(char array1[100][32], char array2[100][32], int arraySize);

int x, minX;
bool interruptHappened = false;

int numItems = 1;
int state = 0;
char menuItems[100][32] = { "Passwords", "Long message", "Very long message", "Very very long message", "short" };
int numState0 = 1;
char tempPass[16] = "";
uint8_t numBlocks = 0;

char state0[1][32] = { "Enter the master password" };


int numState1 = 39;
char state1[50][32] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "@", ".", "OK" };
char state1Input[32] = "";
int numState2 = 1;
char state2[1][32] = { "Password incorrect" };

int numState3 = 5;
char state3[5][32] = { "Login", "Add", "Backup", "Reset", "Exit" };
int value = 0;

int numState4 = 1;
char state4[100][32] = {};

int numState5 = 1;
char state5[1][32] = { "Enter website name" };

int numState6 = 39;
char state6[50][32] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "@", ".", "OK" };

int numState7 = 1;
char state7[1][32] = { "Enter user name" };

int numState8 = 39;
char state8[50][32] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "@", ".", "OK" };

int numState9 = 1;
char state9[1][32] = { "Enter password" };

int numState10 = 39;
char state10[50][32] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "@", ".", "OK" };

int numState11 = 1;
char state11[1][32] = { "Password saved" };

/*
0 "Enter master pass"
1 Take master pass input. onclick check master
2 "Incorrect" (one more variable for keeping track of number of incorrect)
3 "login", "add", "backup", "reset", "exit"
4 website name (login menu)
5 "enter website"
6 take website
7 "enter uname"
8 take uname input
9 "enter pass"
10 take pass

*/
bool testMaster(AES256* aes256, byte* key_hash, const struct cipherVector* testCipher) {
  //test
  aes256->decryptBlock(decryptedText, testCipher->cipher);
  bool valid = true;
  for (int i = 0; i < KEY_SIZE; i++) {
    valid &= decryptedText[i] == testCipher->id[i];
  }
  Serial.print("Master password is ");
  Serial.print(int(valid));
  return valid;
}
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  setupDisplay();
  setupRotaryEncoder();
  setupFlash();
  copyArray(menuItems, state0, numState0);
  delay(2000);
}



void loop() {
  // uint8_t fgt = 0;
  //   i2ceeprom.write((uint16_t)0, &fgt, (uint16_t)1);
  // byte number[64] = {0};
  // i2ceeprom.write(0, number, sizeof(number));
  // delay(50);
  displayMessage(menuItems[value]);
  rotary_loop();
  delay(50);
  // strcpy(f.id, "HELHEHEHELh");
  // writeToFlash(&f,64);
  // readFromFlash(&f, 64);
  // Serial.print("ID:");
  // Serial.println(f.id);
  // delay(500);//remove this

  // aes256.decryptBlock((uint8_t*)thismyoutput, decryptedText);
  // Serial.println((char*)thismyoutput);

  //readFromFlash(&f, (uint16_t)64);
  // Serial.println(f.id);
  // Serial.println(f.name);

  // aes256.decryptBlock(decryptedText,(uint8_t*)f.cipher);
  // Serial.println((char *)decryptedText);
  delay(500);
}
void handleChange() {
  if (state == 0) {
    updateState(1);
    return;
  }
  if (state == 1) {
    if (value == 38) {  //ok
      //Check Master Password
      sha256.reset();
      sha256.update(state1Input, strlen(state1Input));
      sha256.finalize(key_hash, sizeof(key_hash));
      aes256.setKey(key_hash, aes256.keySize());

      byte ecrt[16];
      char passwrd[16];
      strcpy(passwrd, "postersnbd");
      aes256.encryptBlock(f.cipher, (uint8_t*)passwrd);


      if (testMaster(&aes256, key_hash, &cipher1)) {
        strcpy(f.id, "p gmail");
        strcpy(f.name, "posterskensri");
        writeToFlash(&f, (uint16_t)64);
        updateState(3);
        return;
      }

      else {
        strcpy(state1Input, "");
        updateState(0);
        return;
      }

    } else {
      //append character to string
      strcat(state1Input, menuItems[value]);
      Serial.println(state1Input);
    }
  }
  if (state == 3) {
    if (value == 0) {
      updateState(4);
      return;
    }
    if (value == 1) {
      updateState(5);
      return;
    }
  }
  if (state == 4) {
    aes256.decryptBlock(decryptedText, (uint8_t*)f.cipher);
    Serial.println(String(f.name) + "'" + String((char*)decryptedText));
    Serial2.println(String(f.name) + "'" + String((char*)decryptedText));
    //Serial.println((char *)decryptedText);
    return;
  }

  if (state == 5) {
    clearStruct();
    updateState(6);
    return;
  }
  if (state == 6) {
    if (value == 38) {
      updateState(7);
      return;

    } else {
      //append character to string
      strcat(f.id, menuItems[value]);
      Serial.println(state1Input);
    }
  }

  if (state == 7) {
    updateState(8);
    return;
  }
  if (state == 8) {
    if (value == 38) {
      updateState(9);
      return;

    } else {
      //append character to string
      strcat(f.name, menuItems[value]);
      Serial.println(state1Input);
    }
  }

  if (state == 9) {
    updateState(10);
    return;
  }
  if (state == 10) {
    if (value == 38) {
      updateState(11);
      return;

    } else {
      //append character to string
      strcat(tempPass, menuItems[value]);
      Serial.println(state1Input);
    }
  }
  if (state == 11) {
    aes256.encryptBlock(f.cipher, (uint8_t*)tempPass);

    i2ceeprom.read((uint16_t)0, &numBlocks, (uint16_t)1);

    writeToFlash(&f, 64 * (numBlocks + 1));
    numBlocks++;
    i2ceeprom.write((uint16_t)0, &numBlocks, (uint16_t)1);
    updateState(3);
    Serial.println("numblocks " + String(numBlocks));
  }
}

void updateState(int newState) {
  switch (newState) {
    case 0:
      state = 0;
      numItems = numState0;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state0, numItems);
      value = 0;
      break;
    case 1:
      state = 1;
      numItems = numState1;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state1, numItems);
      value = 0;
      break;
    case 2:
      state = 2;
      numItems = numState2;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state2, numItems);
      value = 0;
      break;
    case 3:
      state = 3;
      numItems = numState3;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state3, numItems);
      value = 0;
      break;
    case 4:
      state = 4;
      i2ceeprom.read((uint16_t)0, &numBlocks, (uint16_t)1);
      numItems = numBlocks;
      numState4 = numItems;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state4, numItems);



      for (int i = 0; i < numBlocks; i++) {
        readFromFlash(&data[0], (1 + i) * 64);
        strcpy(menuItems[i], data->id);
        strcat(menuItems[i], "   :   ");
        strcat(menuItems[i], data->name);
      }
      break;
    case 5:
      state = 5;
      numItems = numState5;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state5, numItems);
      break;
    case 6:
      state = 6;
      numItems = numState6;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state6, numItems);
      break;
    case 7:
      state = 7;
      numItems = numState7;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state7, numItems);
      break;
    case 8:
      state = 8;
      numItems = numState8;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state8, numItems);
      break;
    case 9:
      state = 9;
      numItems = numState9;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state9, numItems);
      break;
    case 10:
      state = 10;
      numItems = numState10;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state10, numItems);
      break;
    case 11:
      state = 11;
      numItems = numState11;
      rotaryEncoder.setBoundaries(0, numItems - 1, true);
      copyArray(menuItems, state11, numItems);
      break;
  }
}

void displayMessage(char* message) {
  x = display.width();
  minX = -12 * strlen(message);
  while (x > minX) {
    if (interruptHappened) {
      delay(10);
      interruptHappened = false;
      return;
    }

    display.setCursor(x, 15);
    display.print(message);
    // Serial.print(message);
    // Serial.println(value);
    if (rotaryEncoder.isEncoderButtonClicked())
      rotary_onButtonClick();
    display.display();
    x = x - SCROLLSPEED;
    clearDisplay();
  }
  x = display.width();
}

void clearDisplay() {
  display.clearDisplay();
  topStuff();
}

void topStuff() {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Sat:13  03:56:32  67%");
  display.setTextSize(2);
}

void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.display();
  delay(2000);
  clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
}

void rotary_onButtonClick() {
  static unsigned long lastTimePressed = 0;  // Soft debouncing
  if (millis() - lastTimePressed < 500)
    return;
  lastTimePressed = millis();
  Serial.print("button pressed ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
  handleChange();
}

void rotary_loop() {
  if (rotaryEncoder.encoderChanged()) {
    Serial.print("Value: ");
    Serial.println(rotaryEncoder.readEncoder());
    value = rotaryEncoder.readEncoder();
  }
  if (rotaryEncoder.isEncoderButtonClicked())
    rotary_onButtonClick();
}
void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
  interruptHappened = true;
}

void setupRotaryEncoder() {
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  bool circleValues = true;
  rotaryEncoder.setBoundaries(0, numItems - 1, circleValues);
  rotaryEncoder.disableAcceleration();
}

void copyArray(char array1[100][32], char array2[100][32], int arraySize) {
  for (int j = 0; j < arraySize; j++) {
    strcpy(array1[j], array2[j]);
  }
}

void setupFlash() {
  if (i2ceeprom.begin(0x50, &Wire)) {  // you can stick the new i2c addr in here, e.g. begin(0x51);
    Serial.println("Found I2C EEPROM");
  } else {
    Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1) delay(10);
  }
}

void writeToFlash(struct cipherVector* cipherVectorPtr, uint16_t addr) {
  memcpy(buffer, (void*)cipherVectorPtr, sizeof(cipherVector));  //to write
  i2ceeprom.write(addr, buffer, sizeof(buffer));
}

void readFromFlash(struct cipherVector* cipherVectorPtr, uint16_t addr) {
  i2ceeprom.read(addr, buffer, sizeof(cipherVector));  //to read
  memcpy((cipherVector*)cipherVectorPtr, buffer, sizeof(cipherVector));
}

void clearStruct() {
  strcpy(f.id, "");
  strcpy(f.name, "");
  strcpy(tempPass, "");
}