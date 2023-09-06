#!/usr/bin/env bash

# Check required command line args
if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    echo "Usage: ./run.sh <logical cpu> <sampling interval>"
    exit 1
fi

# Check if venv exists
if [ ! -d "venv" ]; then
    # Create venv if it doesn't exist
    python3 -m venv venv
    # install necessary python3 packages
    source venv/bin/activate
    pip install matplotlib
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
if ! sudo ./build/rapl_threaded $1 $2; then
    # Print error message
    echo "Error running rapl_threaded"
    exit 1
fi

# Plot the results
python3 energy_power.py energy_readings.csv $2 time_slots.txt