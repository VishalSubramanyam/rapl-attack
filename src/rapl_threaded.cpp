#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <functional>
#include <unordered_map>
// Linux-specific headers
#include <bits/chrono.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "computations.hpp"
#include "rapl.hpp"

using clock_used = std::chrono::steady_clock;
using time_point = std::chrono::time_point<clock_used>;

int PINNED_CPU = 0;
int SAMPLING_INTERVAL = 150;  // microseconds
auto GLOBAL_START = clock_used::now();

bool victimDone = false;
bool spyMonitoringStarted = false;
time_point spyGraphStartTime;

// time point variable pair for storing the time when intense computation start
// and end
std::pair<time_point, time_point> intenseComputationTime;
// time point variable pair for storing the time when empty computation start
// and end
std::pair<time_point, time_point> emptyComputationTime;

void* victimThread(void* computationNames) {
    auto const& computationStrings = *static_cast<std::vector<std::string> *>(computationNames);
    // set affinity of this process to PINNED_CPU
    set_affinity(PINNED_CPU);
    // don't start executing until spy thread has started recording the energy
    // values
    while (!spyMonitoringStarted) {
        // yield to the spy thread
        sched_yield();
    }
    Computation::init(spyGraphStartTime, SAMPLING_INTERVAL);
    // delete time_slots.txt file
    remove("time_slots.txt");

    // create a vector of computations
    std::vector<Computation> computations;
    for (auto const& computationName : computationStrings) {
	std::cout << "DEBUG: Computation Name -> " << computationName << std::endl;
        computations.emplace_back(computationParser.at(computationName)());
    }

    // execute every computation
    for (auto& comp : computations) {
        comp.performComputation();
    }
    // set victimDone to true
    victimDone = true;

    // print DONE
    printf("Victim DONE\n");
    return nullptr;
}

void* spyThread(void*) {
    // pin this process to a different CPU than that of the victim to prevent
    // interference of any kind
    if (PINNED_CPU != 0) {
        set_affinity(0);
    } else {
        set_affinity(1);
    }
    // Monitor the energy consumption of the child process at regular intervals

    // Sampling interval
    std::chrono::microseconds interval(SAMPLING_INTERVAL);
    std::vector<double> energyConsumed;
    auto startTimeCurrentInterval = clock_used::now();
    spyGraphStartTime = startTimeCurrentInterval;
    while (true) {
        // check if child process has exited
        // TODO: Will the following if statement pollute the energy data?
        if (victimDone) {
            break;
        }
        // get energy consumed
        energyConsumed.push_back(getEnergyConsumed(PINNED_CPU));
        auto endTime = clock_used::now();
        // busy wait for the rest of the interval
        do {
            endTime = clock_used::now();
        } while (endTime - startTimeCurrentInterval < interval);
        startTimeCurrentInterval += interval;
        // if we were context switched out, insert -1 into energyConsumed for
        // every time slot missed
        while (endTime - startTimeCurrentInterval > interval) {
            energyConsumed.push_back(0);
            startTimeCurrentInterval += interval;
        }
        spyMonitoringStarted = true;
    }
    // output energy consumed to energy_readings.csv
    auto energyFile = fopen("energy_readings.csv", "w");
    for (auto& energy : energyConsumed) {
        fprintf(energyFile, "%lf\n", energy);
    }
    return nullptr;
}

int main(int argc, char** argv) {
    // arg count check
    if (argc != 4) {
        printf("Usage: %s <PINNED_CPU> <SAMPLING_INTERVAL> <ARGS,>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // argv[1] = PINNED_CPU
    PINNED_CPU = atoi(argv[1]);
    // argv[2] = SAMPLING_INTERVAL
    SAMPLING_INTERVAL = atoi(argv[2]);

    // argv[3] = computationNames separated by commas
    std::vector<std::string> computationNames;
    // Create a string stream from the input string
    std::istringstream iss(argv[3]);
    std::string token;
    while (std::getline(iss, token, ',')) {
        // Add each parsed name to the vector
        computationNames.push_back(token);
    }

    // create a monitoring thread using pthread_create
    pthread_t spyThreadId;
    pthread_create(&spyThreadId, NULL, &spyThread, nullptr);

    // create a thread using pthread_create
    pthread_t victimThreadId;
    pthread_create(&victimThreadId, NULL, &victimThread, &computationNames);

    // wait for the victim thread to finish
    pthread_join(victimThreadId, NULL);
    // wait for the monitoring thread to finish
    pthread_join(spyThreadId, NULL);
    return 0;
}
