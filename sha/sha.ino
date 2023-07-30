#include <Crypto.h>
#include <SHA256.h>
#include <string.h>

#define HASH_SIZE 32
char value[HASH_SIZE];

struct TestHashVector {
  const char *name;
  const char *key;
  const char *data;
  uint8_t hash[HASH_SIZE];
};

static TestHashVector const testVectorSHA256_1 = {
  "SHA-256 #1",
  0,
  "abc",
  { 0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
    0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
    0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
    0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad }
};
static TestHashVector const testVectorSHA256_2 = {
  "SHA-256 #2",
  0,
  "lmao",
  { 0x90, 0xBF, 0xFE, 0x18, 0x84, 0xB8, 0x4D, 0x5E,
    0x25, 0x5F, 0x12, 0xFF, 0x0E, 0xCB, 0xD7, 0x0F,
    0x2E, 0xDF, 0xC8, 0x77, 0xB6, 0x8D, 0x61, 0x2D,
    0xC6, 0xFB, 0x50, 0x63, 0x8B, 0x3A, 0xC1, 0x7C }
};

SHA256 sha256;


void testHash(Hash *hash, const struct TestHashVector *test) {
  bool ok = true;

  Serial.print(test->name);
  Serial.print(" ... ");

  size_t size = strlen(test->data);
  size_t posn, len;

  hash->reset();
  hash->update(test->data, size);
  // for (posn = 0; posn < size; posn += inc) {
  //     len = size - posn;
  //     if (len > inc)
  //         len = inc;
  //     Serial.print("lol");
  // }
  hash->finalize(value, sizeof(value));
  for (int lmao = 0; lmao < HASH_SIZE; lmao++) {
    Serial.print("0x");
    Serial.print(value[lmao], HEX);
    // Serial.print(".");
    // Serial.print(int(value[lmao] == test->hash[lmao]));
    Serial.print(", ");
    ok &= value[lmao] == test->hash[lmao];
  }
  // if (memcmp(value, test->hash, sizeof(value)) != 0)
  //   ok = false;

  // ok = true;

  if (ok)
    Serial.println("Passed");
  else
    Serial.println("Failed");
}


void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println();

  Serial.print("State Size ...");
  Serial.println(sizeof(SHA256));
  Serial.println();

  Serial.println("Test Vectors:");
  testHash(&sha256, &testVectorSHA256_1);
  testHash(&sha256, &testVectorSHA256_2);

  sha256.reset();
  char msg[] = "lmao";
  sha256.update(msg, strlen(msg));
  sha256.finalize(value, sizeof(value));
  for (int i = 0; i < HASH_SIZE; i++) {
    Serial.print(value[i], HEX);
  }

  Serial.println();
}

void loop() {
}
