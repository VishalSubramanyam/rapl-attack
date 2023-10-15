import numpy as np
from extract_readings import extract_readings
computationPowerMap1, power1 = extract_readings('aesOpenSSLKeyFixedPtFixed.csv', 'aesOpenSSLKeyFixedPtFixed.time.txt')
computationPowerMap2, power2 = extract_readings('aesOpenSSLKeyVariesPtFixed.csv', 'aesOpenSSLKeyVariesPtFixed.time.txt')
# each map above contains one key-value pair, where key is the name of a computation, and the value is a tuple of two ints
# the first int is the starting slot number, and the second int is the ending slot number

# extract the power readings for the first computation
readings1 = computationPowerMap1['aesOpenSSLKeyFixedPtFixed']
# extract the power readings for the second computation
readings2 = computationPowerMap2['aesOpenSSLKeyVariesPtFixed']
# perform a t-test between the two lists of power readings
import scipy.stats as stats
print(f"t-test value: {stats.ttest_ind(readings1, readings2).statistic}")