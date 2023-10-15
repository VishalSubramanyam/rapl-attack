#!/usr/bin/env bash
SCRIPTS_FOLDER=scripts
# Check required command line args
if [ "$#" -le 3 ]; then
    echo "Illegal number of parameters"
    echo "Usage: ./run.sh <logical cpu> <sampling interval> <t_test/t_test_separate/graph/cpa> <computations...>"
    exit 1
fi
LOGICAL_CPU=$1
SAMPLING_INTERVAL=$2
MODE=$3
# Check if venv exists
if [ ! -d "venv" ]; then
    # Create venv if it doesn't exist
    python3 -m venv venv
    # install necessary python3 packages
    source venv/bin/activate
    pip install matplotlib
    # install scipy
    pip install scipy
    # Check if python3-tk is installed (assuming Ubuntu)
    if ! dpkg -s python3-tk >/dev/null 2>&1; then
        # Install python3-tk if it isn't installed
        sudo apt-get install -y python3-tk
    fi
fi
source venv/bin/activate

# Make build directory if it doesn't exist
mkdir -p build

# Use Cmake to build the project into build dir
cd build
cmake ..
cmake --build .
cd ..

# Run the executable
# Format: ./build/rapl_threaded <logical cpu> <sampling interval>
# logical cpu: The cpu to pin the victim process
# sampling interval: The interval in microseconds to sample the power consumption
# Take the arguments from the first two arguments of the script
# Check exit code of the executable run with sudo
# if ! sudo ./build/rapl_threaded $1 $2 empty,aesniKeyFixedPtVaries,empty,aesniPtFixedKeyVaries; then
#     # Print error message
#     echo "Error running rapl_threaded"
#     exit 1
# fi

if [[ $MODE == "t_test_separate" ]]; then
    for((i=4; i<=$#; i++)); do
        arg="${!i}"
        sudo ./build/rapl_threaded $1 $2 $arg
        sudo mv energy_readings.csv $arg.csv
        sudo mv time_slots.txt $arg.time.txt
    done
else
    arg="${4}"
    for((i=5; i<=$#; i++)); do
        arg+=",${!i}"
    done
    sudo ./build/rapl_threaded $1 $2 $arg
fi

# Plot the results
if [[ $MODE == "graph" ]]; then
    python3 $SCRIPTS_FOLDER/energy_power.py energy_readings.csv time_slots.txt
elif [[ $MODE == "t_test" ]]; then
    python3 $SCRIPTS_FOLDER/t_test.py energy_readings.csv time_slots.txt aesOpenSSLKeyFixedPtFixed aesOpenSSLKeyVariesPtFixed
elif [[ $MODE == "t_test_separate" ]]; then
    # Following script assumes aesniKeyFixedPtFixed.csv and aesniKeyVariesPtFixed.csv exist
    python3 $SCRIPTS_FOLDER/t_test_separate.py
elif [[ $MODE == "cpa" ]]; then
    python3 $SCRIPTS_FOLDER/cpa.py energy_readings.csv
else
    echo "Invalid argument - Expected t_test/graph/cpa"
    exit 1
fi
