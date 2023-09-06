# read input file from first argument
# Each line contains one number
# Read each number into a list

import sys
import math
import numpy as np
# Read input file
if len(sys.argv) != 4:
    print("Usage: python3 energy_power.py <input file> <sampling interval> <time file>")
    sys.exit(1)

# Read input file
input_file = sys.argv[1]
with open(input_file, "r") as f:
    lines = f.readlines()

# Convert each line into a floating value
energy = []
for line in lines:
    energy.append(float(line))

# The energy list contains energy readings at successive intervals
# Whenever energy couldn't be read, it contains -1
# Construct power list
power = []
for i in range(1, len(energy)):
    if energy[i] == 0 or energy[i-1] == 0:
        power.append(0)
    else:
        power.append(energy[i] - energy[i-1])

# second command-line arg is the sampling interval
# divide power readings by sampling interval (in microseconds) to get power
sampling_interval = int(sys.argv[2]) * math.pow(10, -6)
for i in range(len(power)):
    power[i] = power[i] / sampling_interval

# Read the start and end time points for every type of computation
# Each line is of the format:
#       computation_name startTimeSlot endTimeSlot
# Compute the mean, median of the power readings for each computation in from start to end

time_file = sys.argv[3]
with open(time_file, "r") as f:
    lines = f.readlines()

# Construct a dictionary of computation names and their start and end time slots
# The dictionary is of the format:
#       computation_name: [startTimeSlot, endTimeSlot]
computation_times = {}
for line in lines:
    line = line.split()
    computation_times[line[0]] = [int(line[1]), int(line[2])]

# Compute the mean, median of the power readings for each computation in from start to end
# Construct a dictionary of computation names and their mean and median power readings
# The dictionary is of the format:
#       computation_name: [mean, median]
computation_power = {}
for computation in computation_times:
    # Get the start and end time slots for the computation
    start_time = computation_times[computation][0]
    end_time = computation_times[computation][1]
    # Get the power readings for the computation
    power_readings = power[start_time:end_time]
    # Compute the mean and median of the power readings
    mean = np.mean(power_readings)
    median = np.median(power_readings)
    # Add the mean and median to the dictionary
    computation_power[computation] = [mean, median]

# Print the mean and median power readings for each computation with proper labels
for computation in computation_power:
    print(computation + ":")
    print("Mean: " + str(computation_power[computation][0]))
    print("Median: " + str(computation_power[computation][1]))
    print()

# Each power reading is made after an interval of "sampling_interval" microseconds
# Plot power readings against time, noting that power list indices don't indicate time
# Construct time list
time = []
for i in range(len(power)):
    time.append(i * sampling_interval * math.pow(10, 6))

# Plot power readings against time
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('TKagg')
# Plot the graph with red markers on actual datapoints and lines connecting the markers
# should be in black
plt.plot(time, power, 'ro', time, power, 'k')
#plt.plot(time, power)
plt.xlabel("Time (microseconds)")
plt.ylabel("Power (Watts)")
plt.title("Power vs Time")
plt.show()

# for every computation type, plot the frequency chart of power readings based on their time slots
# Construct a dictionary of computation names and their power readings
# The dictionary is of the format:
#       computation_name: [power readings]
computation_power = {}

for computation in computation_times:
    # Get the start and end time slots for the computation
    start_time = computation_times[computation][0]
    end_time = computation_times[computation][1]
    # Get the power readings for the computation
    power_readings = power[start_time:end_time]
    # Add the power readings to the dictionary
    computation_power[computation] = power_readings

# Plot the frequency chart of power readings for each computation
for computation in computation_power:
    # Get the power readings for the computation
    power_readings = computation_power[computation]
    # Plot the frequency chart of power readings for the computation
    plt.hist(power_readings, bins=200)
plt.xlabel("Power (Watts)")
plt.ylabel("Frequency")
plt.title("Frequency Chart of Power Readings")
# legend
plt.legend(computation_power.keys())
plt.show()

