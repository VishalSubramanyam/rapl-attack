#pragma once
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <wmmintrin.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#define ALIGNED16 __attribute__((aligned(16)))
#define AES_KEY_SIZE 16

namespace customAES {
std::vector<uint8_t> aes128_encrypt_cbc_openssl(
    const std::vector<uint8_t> &plaintext, const char key[AES_KEY_SIZE]
);

// function that takes 128-bit key as input and returns 11 round keys
// through its second parameter
// Addresses needed to be 16-byte aligned (for speed)
void set_encrypt_key(__m128i const *key, __m128i *roundKeys);

std::vector<uint8_t> aes128_encrypt_cbc_aesni(
    const std::vector<uint8_t> &plaintext, const char key[AES_KEY_SIZE]
);
}  // namespace customAES
