# read input file from first argument
# Each line contains one number
# Read each number into a list

import sys
import math
import numpy as np
def extract_readings(energyFile, timeFile):
    with open(energyFile, "r") as f:
        lines = f.readlines()
    energy = [float(line) for line in lines]
    power = []
    for i in range(1, len(energy)):
        if energy[i] == 0 or energy[i-1] == 0:
            power.append(0)
        else:
            power.append(energy[i] - energy[i-1])
    # read time file
    # each line in time file -> computationName starting_slot ending_slot
    with open(timeFile) as f:
        lines = f.readlines()
    # create a dictionary that maps each computation name to a tuple of starting and ending slot numbers
    computationPowerMap = {}
    for line in lines:
        temp = line.split()
        computationName = temp[0]
        startingSlot = int(temp[1])
        endingSlot = int(temp[2])
        computationPowerMap[computationName] = power[startingSlot : endingSlot + 1]
    return computationPowerMap, power
