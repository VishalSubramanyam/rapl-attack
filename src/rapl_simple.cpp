#include <climits>
#include <cstdint>
#include <iostream>
// Linux-specific headers
#include <fcntl.h>
#include <unistd.h>

#include "rapl.hpp"

#define PINNED_CPU 10



void intenseComputation(){
    double sum = 0;
    for (int i = 0; i < 1e9; i++) {
        sum += 10;
        sum -= 5;
        sum /= 3.0;
    }
}

int main() {
    int a[] = {1,2,3};
    int b[] = {4,5,6};
    
    // print the PID of this process
    printf("PID: %d\n", getpid());

    // set affinity of this process to a single core (core PINNED_CPU)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(PINNED_CPU, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
    // get energy consumed before and after a long computation for every CPU
    std::vector<double> energyStarts, energyEnds;
    for (int i = 0; i < sysconf(_SC_NPROCESSORS_CONF); i++) {
        energyStarts.push_back(getEnergyConsumed(i));
    }
    intenseComputation();
    for (int i = 0; i < sysconf(_SC_NPROCESSORS_CONF); i++) {
        energyEnds.push_back(getEnergyConsumed(i));
    }
    // print the energy consumed by each CPU along with delta
    for (int i = 0; i < sysconf(_SC_NPROCESSORS_CONF); i++) {
        printf(
            "CPU %d: %lf joules\n", i, (energyEnds[i] - energyStarts[i])
        );
    }
    fflush(stdout);
}