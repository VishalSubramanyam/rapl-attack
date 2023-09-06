#include "computations.hpp"

#include <immintrin.h>
#include <openssl/aes.h>

#include <string>
#include <vector>

#include "aes.hpp"
#define LOOP_ITER 1e7
extern "C" {
void emptyComputation() {
    for (int i = 0; i < 100 * LOOP_ITER; i++) {
        // do nothing
    }
}
void aesOpenSSLComputation() {
    // perform AES computation using the function from aes.hpp
    // use some plaintext and some secret key
    std::string plaintext = "This is a plaintext";
    char key[AES_KEY_SIZE] = "123456789123456";
    for (int i = 0; i < LOOP_ITER; i++) {
        std::vector<uint8_t> ciphertext = customAES::aes128_encrypt_cbc_openssl(std::vector<uint8_t>(plaintext.begin(), plaintext.end()), key);
    }
}   

extern void aesniComputation(){
    std::string plaintext = "This is a plaintext";
    char key[AES_KEY_SIZE] = "123456789123456";
    for (int i = 0; i < LOOP_ITER; i++) {
        std::vector<uint8_t> ciphertext = customAES::aes128_encrypt_cbc_aesni(std::vector<uint8_t>(plaintext.begin(), plaintext.end()), key);
    }
}


void avx2Computation() {
    __m256i a = _mm256_set_epi32(8, 7, 6, 5, 4, 3, 2, 1);
    __m256i b = _mm256_set_epi32(1, 1, 1, 1, 1, 1, 1, 1);
    asm("vmovdqa ymm0, %0\n\t"
        "vmovdqa ymm1, %1\n\t"
        :
        : "m"(a), "m"(b)
        : "ymm0", "ymm1");
    // Run only the vpaddd instruction inside a loop. The vmovdqu instructions
    // should be placed outside the loop
    for (int i = 0; i < LOOP_ITER; i++) {
        asm("vdivpd ymm0, ymm0, ymm1\n\t" : : : "ymm0", "ymm1");
    }
    asm("vmovdqu %0, ymm0\n\t" : "=m"(a) : : "ymm0");
}
void mixedSIMDComputation() {
    // use a mix of 128 and 256 bit registers
    __m128i arrayA_128bit = _mm_set_epi32(4, 3, 2, 1);
    __m128i arrayB_128bit = _mm_set_epi32(1, 1, 1, 1);
    __m256i a = _mm256_set_epi32(8, 7, 6, 5, 4, 3, 2, 1);
    __m256i b = _mm256_set_epi32(1, 1, 1, 1, 1, 1, 1, 1);
    asm("vmovdqa ymm0, %0\n\t"
        "vmovdqa ymm1, %1\n\t"
        :
        : "m"(a), "m"(b)
        : "ymm0", "ymm1");
    asm("vmovdqu xmm2, %0\n\t"
        "vmovdqu xmm3, %1\n\t"
        :
        : "m"(arrayA_128bit), "m"(arrayB_128bit)
        : "xmm2", "xmm3");
    for (int i = 0; i < LOOP_ITER; i++) {
        asm("divpd xmm2, xmm3\n\t" : : : "xmm0", "xmm1");
        asm("vdivpd ymm0, ymm0, ymm1\n\t" : : : "ymm0", "ymm1");
    }
}
}
time_point Computation::spyStartTime;
uint Computation::samplingInterval;
bool Computation::isInitialized = false;