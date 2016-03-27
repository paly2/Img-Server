#ifndef BLOWFISH_H
#define BLOWFISH_H

#define PASSWORD_LEN 50
#define BASE64_KEYSTR "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="
#define INDEX_HTML_PASSWORD_LINE 26
#include <inttypes.h>

typedef struct {
  uint32_t P[16 + 2];
  uint32_t S[4][256];
} BLOWFISH_CTX;

void Blowfish_Init(BLOWFISH_CTX *ctx, uint8_t *key, int32_t keyLen);
void Blowfish_Encrypt(BLOWFISH_CTX *ctx, uint32_t *xl, uint32_t *xr);
void Blowfish_Decrypt(BLOWFISH_CTX *ctx, uint32_t *xl, uint32_t *xr);

void generate_password(char* password);
void decode_base64(uint8_t* output, const uint8_t* input);
void decrypt(char* output, const char* key, const char* input);

#endif
