#include <Crypto.h>
#include <SHA256.h>
#include <AES.h>
#include <string.h>


#define HASH_SIZE 32
#define KEY_SIZE 16

byte key_hash[HASH_SIZE];
byte decryptedText[KEY_SIZE];

SHA256 sha256;
AES256 aes256;

struct cipherVector {
  const char* name;       //name of id, password
  const char* id;         //contains userid or email
  byte cipher[KEY_SIZE];  //encrypted password
};

// struct hashVector {      //test for verifying if master password is correct
//   const char* data;      //unhashed data
//   byte hash[HASH_SIZE];  //hashed data
// };

// hashVector const hashTest1 = {
//   "1234567890"
// }

cipherVector const testCipher1 = {
  //to test master password
  "Test",
  "123456789012345",
  { 0x5D, 0x16, 0x26, 0x6E, 0x10, 0xB8, 0xE5, 0xFE,
    0x76, 0x4C, 0x3A, 0x2B, 0xD6, 0x80, 0xAE, 0xF6 }  //123456789012345
};

cipherVector const cipher1 = {
  "Gmail",
  "posterskensri@gmail.com",
  { 0x5D, 0x16, 0x26, 0x6E, 0x10, 0xB8, 0xE5, 0xFE,
    0x76, 0x4C, 0x3A, 0x2B, 0xD6, 0x80, 0xAE, 0xF6 }  //123456789012345
};

// //plaintext[32] contain the text we need to encrypt
// byte plaintext[KEY_SIZE] = "123456789012345";
// //cipher[HASH_SIZE] stores the encrypted text
// byte cipher[KEY_SIZE];
// //decryptedtext[HASH_SIZE] stores decrypted text after decryption
// byte decryptedtext[KEY_SIZE];


void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  char master[] = "lmaolmao";
  sha256.reset();
  sha256.update(master, strlen(master));
  sha256.finalize(key_hash, sizeof(key_hash));
  aes256.setKey(key_hash, aes256.keySize());  // Setting Key for AES, uses hash of master password as key for aes

  // Serial.print("Before Encryption:");
  // for (int i = 0; i < KEY_SIZE; i++) {
  //   Serial.write(plaintext[i]);
  // }

  // aes256.encryptBlock(cipher, plaintext);  //cipher->output block and plaintext->input block
  // Serial.println();
  // Serial.print("After Encryption:");
  // for (int j = 0; j < KEY_SIZE; j++) {
  //   Serial.print("0x");
  //   Serial.print((cipher[j]), HEX);
  //   Serial.print(", ");
  // }

  testMaster(&aes256, key_hash, &testCipher1);

  Serial.println();
  Serial.print("After Decryption:");
  for (int i = 0; i < KEY_SIZE; i++) {
    Serial.write(decryptedText[i]);
  }
  Serial.println();
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
}

bool testMaster(AES256* aes256, byte* key_hash, const struct cipherVector* testCipher){
  //test
  aes256->decryptBlock(decryptedText, testCipher->cipher);
  bool valid = true;
  for(int i = 0; i < KEY_SIZE; i++){
    valid &= decryptedText[i] == testCipher->id[i];
  }
  Serial.print("Master password is ");
  Serial.print(int(valid));
  return valid;
}
