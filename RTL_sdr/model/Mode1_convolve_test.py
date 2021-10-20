from os import name
import numpy as np
from scipy import signal

expander = 24
decimator = 125

def convolve(x, h, s):
    y = np.zeros(len(x) + len(h)-1)
    for i in range(len(y)):
        for j in range(len(h)):
            if(i-j >= 0 and i-j < len(x)):
                y[i] += h[j] * x[i-j]
    return y[::s]


def convolve_compressed(x, h, expander, decimator):
    ouput_s = int((expander*x.size+h.size-1) / decimator) + 1
    y = np.zeros(ouput_s)
    y_index = 0; x_index = 0
    i = 0
    while(i < x.size * 24):
        for j in range(len(h)):
            if(i -j >= 0 and (i-j) %24 == 0):
                y[y_index] += h[j] * x[int((i-j)/24)]

        i += decimator
        y_index += 1
    return y



if __name__ == "__main__":
    #construct test case
    n_taps = 11
    sample_size = 50
    audio_coeff = np.random.randint(20,size=n_taps)

    sample = np.random.randint(100,size=sample_size)
    sample_expanded = np.zeros(sample_size*expander)
    
    k = 0
    for i in range(0,sample.size):
        sample_expanded[k] = sample[i]
        k += 24

    x = signal.lfilter(audio_coeff,1.0,sample_expanded)
    x = x[::decimator]
    y = convolve(sample_expanded,audio_coeff,125)
    z = convolve_compressed(sample,audio_coeff,24,125)
    x==z

