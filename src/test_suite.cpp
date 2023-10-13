#include <immintrin.h>
#include <stdio.h>

#include <chrono>
#include <vector>

#include "aes.hpp"
#include "computations.hpp"

// test if key scheduling works correctly against OpenSSL's AES_set_encrypt_key
// function
bool testKeyScheduling() {
    // generate a random 128-bit key
    ALIGNED16 char key[AES_KEY_SIZE];
    RAND_bytes(reinterpret_cast<uint8_t *>(key), AES_BLOCK_SIZE);
    // generate round keys using the customAES::set_encrypt_key function
    uint8_t roundKeys[11 * 16] ALIGNED16;
    customAES::set_encrypt_key(
        reinterpret_cast<__m128i const *>(key),
        reinterpret_cast<__m128i *>(roundKeys)
    );
    // generate round keys using the OpenSSL AES_set_encrypt_key function
    AES_KEY aes_enc_key;
    AES_set_encrypt_key(
        reinterpret_cast<const uint8_t *>(key), AES_BLOCK_SIZE * 8, &aes_enc_key
    );
    // check if all the round keys are the same
    for (int i = 0; i < 11; i++) {
        __m128i roundKey = reinterpret_cast<__m128i *>(roundKeys)[i];
        __m128i opensslRoundKey =
            reinterpret_cast<__m128i *>(aes_enc_key.rd_key)[i];
        if (_mm_movemask_epi8(_mm_cmpeq_epi8(roundKey, opensslRoundKey)) !=
            0xFFFF) {
            return false;
        }
    }
    return true;
}

// test AES encryption against OpenSSL's AES_cbc_encrypt function
bool testAESEncryption() {
    std::string plaintext = "This is a plaintext";
    char key[AES_KEY_SIZE] = "123456789123456";
    // encrypt using OpenSSL's AES_cbc_encrypt function
    auto openSSLResult = customAES::aes128_encrypt_cbc_openssl(
        std::vector<uint8_t>(plaintext.begin(), plaintext.end()), key
    );
    auto customResult = customAES::aes128_encrypt_cbc_aesni(
        std::vector<uint8_t>(plaintext.begin(), plaintext.end()), key
    );
    // check if the results are the same
    if (openSSLResult.size() != customResult.size()) {
        return false;
    }
    for (int i = 0; i < openSSLResult.size(); i++) {
        if (openSSLResult[i] != customResult[i]) {
            // print both ciphertexts
            printf("OpenSSL ciphertext: ");
            for (int j = 0; j < openSSLResult.size(); j++) {
                printf("%02x", openSSLResult[j]);
            }
            printf("\n");
            printf("Custom ciphertext:  ");
            for (int j = 0; j < customResult.size(); j++) {
                printf("%02x", customResult[j]);
            }
            printf("\n");
            return false;
        }
    }
    return true;
}

int main() {
    // test key scheduling
    if (!testKeyScheduling()) {
        printf("Key scheduling test failed\n");
        return 1;
    }
    printf("Key scheduling test passed\n");
    // test AES encryption
    if (!testAESEncryption()) {
        printf("AES encryption test failed\n");
        return 1;
    }
    printf("AES-NI encryption code works correctly (compared against OpenSSL)\n");
    

    return 0;
}
