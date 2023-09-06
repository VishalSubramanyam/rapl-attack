#pragma once
#include <openssl/aes.h>

#include <chrono>
#include <map>
#include <stdexcept>
#include <string>
using clock_used = std::chrono::steady_clock;
using time_point = std::chrono::time_point<clock_used>;

extern "C" {
extern void aesOpenSSLComputation();
extern void emptyComputation();
extern void avx2Computation();
extern void mixedSIMDComputation();
extern void aesniComputation();
}

class Computation {
    static time_point spyStartTime;
    static uint samplingInterval;
    static bool isInitialized;
    uint startTimeSlot;
    uint endTimeSlot;

   public:
    std::string name;
    void (*compFuncPtr)(void);
    static void init(time_point const &spyStart, uint samplingInt) {
        spyStartTime = spyStart;
        samplingInterval = samplingInt;
        isInitialized = true;
    }
    /**
     * @brief Construct a new Computation object
     *
     * @param name Name of the computation
     * @param func Function pointer to the computation
     * @param spyStartTime Time point when the spy thread started recording the
     * energy values
     * @param samplingInterval Sampling interval in microseconds
     */
    Computation(std::string name, void (*func)(void)) {
        if (!isInitialized) {
            throw std::runtime_error("Computation class not initialized");
        }
        this->name = name;
        this->compFuncPtr = func;
    }
    void performComputation() {
        printf("Starting computation %s\n", this->name.c_str());
        time_point startTime = clock_used::now();
        this->compFuncPtr();
        time_point endTime = clock_used::now();
        // convert to slots based on spyStartTime and samplingInterval
        this->startTimeSlot =
            std::chrono::duration_cast<std::chrono::microseconds>(
                startTime - spyStartTime
            )
                .count() /
            samplingInterval;
        this->endTimeSlot =
            std::chrono::duration_cast<std::chrono::microseconds>(
                endTime - spyStartTime
            )
                .count() /
            samplingInterval;
        // open time_slots.txt in append mode
        FILE *fp = fopen("time_slots.txt", "a");
        fprintf(
            fp, "%s %d %d\n", this->name.c_str(), this->startTimeSlot,
            this->endTimeSlot
        );
        fclose(fp);
        printf("Ending computation %s\n", this->name.c_str());
        fflush(stdout);
    }
};