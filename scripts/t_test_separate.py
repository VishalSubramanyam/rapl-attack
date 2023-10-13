import numpy as np

with open('aesniKeyFixedPtFixed.csv') as f:
    lines=f.readlines()
readings1 = np.diff([float(line) for line in lines])

with open('aesniKeyVariesPtFixed.csv') as f:
    lines=f.readlines()
readings2 = np.diff([float(line) for line in lines])

import scipy.stats as stats
print(f"t-test value: {stats.ttest_ind(readings1, readings2).statistic}")