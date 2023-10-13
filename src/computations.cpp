#include "computations.hpp"

#include <fcntl.h>
#include <immintrin.h>
#include <openssl/aes.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "aes.hpp"
#define LOOP_ITER 1e7
time_point Computation::spyStartTime;
uint Computation::samplingInterval;
bool Computation::isInitialized = false;
static std::vector<std::vector<char>> pregeneratedKeys;
void Computation::init(const time_point &spyStart, uint samplingInt) {
    spyStartTime = spyStart;
    samplingInterval = samplingInt;
    isInitialized = true;
    // fill in pregenerated keys
    int fd = open("/dev/random", O_RDONLY);
    for (int i = 0; i < LOOP_ITER; i++) {
        std::vector<char> temp(16);
        int res = read(fd, temp.data(), 16);
        if (res < 0)
            throw std::runtime_error(
                std::string(
                    "Couldn't read from /dev/random during key generation -- "
                ) +
                strerror(errno)
            );
        pregeneratedKeys.emplace_back(temp);
    }
}

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
        std::vector<uint8_t> ciphertext = customAES::aes128_encrypt_cbc_openssl(
            std::vector<uint8_t>(plaintext.begin(), plaintext.end()), key
        );
    }
}

void aesniComputation() {
    std::string plaintext = "This is a plaintext";
    char key[AES_KEY_SIZE] = "123456789123456";
    for (int i = 0; i < LOOP_ITER; i++) {
        std::vector<uint8_t> ciphertext = customAES::aes128_encrypt_cbc_aesni(
            std::vector<uint8_t>(plaintext.begin(), plaintext.end()), key
        );
    }
}

void aesniKeyFixedPtFixed() {
    char key[AES_KEY_SIZE] = "123456789123456";
    // read 16 bytes from /dev/random
    auto fd = open("/dev/random", O_RDONLY);
    uint8_t plaintext[AES_BLOCK_SIZE];
    read(fd, plaintext, AES_BLOCK_SIZE);
    for (int i = 0; i < LOOP_ITER; i++) {
        std::vector<uint8_t> ciphertext = customAES::aes128_encrypt_cbc_aesni(
            std::vector<uint8_t>(plaintext, plaintext + AES_BLOCK_SIZE), key
        );
    }
    close(fd);
}

void aesniKeyVariesPtFixed() {
    std::string plaintext = "123456789123456";
    auto fd = open("/dev/random", O_RDONLY);
    for (int i = 0; i < LOOP_ITER; i++) {
        std::vector<uint8_t> ciphertext = customAES::aes128_encrypt_cbc_aesni(
            std::vector<uint8_t>(plaintext.begin(), plaintext.end()), pregeneratedKeys[i].data()
        );
    }
    close(fd);
}

void aesniCPA() {
    std::string key = "123456789123456";
    int const NUM_PLAINTEXTS = 1e6;
    std::vector<std::string> plaintexts(NUM_PLAINTEXTS, std::string(16, '\0'));
    auto fd = open("/dev/random", O_RDONLY);
    for (int i = 0; i < NUM_PLAINTEXTS; i++) {
        read(fd, plaintexts[i].data(), 16);
    }
    std::vector<std::vector<uint8_t>> ciphertexts;
    close(fd);
    for (int i = 0; i < NUM_PLAINTEXTS; i++) {
        std::vector<uint8_t> ciphertext = customAES::aes128_encrypt_cbc_aesni(
            std::vector<uint8_t>(
                plaintexts[i].data(), plaintexts[i].data() + 16
            ),
            key.c_str()
        );
        ciphertexts.push_back(std::move(ciphertext));
    }
    // put the plaintexts and ciphertexts in a file
    std::ofstream fout("aesniCPA.bin", std::ios::binary);
    fout.write(key.data(), key.size());
    for (int i = 0; i < NUM_PLAINTEXTS; i++) {
        fout.write(
            reinterpret_cast<char const *>(ciphertexts[i].data()),
            ciphertexts[i].size()
        );
    }
    fout.close();
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
