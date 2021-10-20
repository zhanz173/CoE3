import numpy as np
import math
from scipy import signal
import matplotlib.pyplot as plt

Fb = 18.5e3
Fe = 19.5e3
Fs = 240e3
Ntaps = 51

def banpass(fb, fe, fs, Ntaps):
	norm_center = (fb+fe)/fs
	norm_pass = (fe-fb)/(fs/2)
	t = norm_pass/2
	h = np.zeros(Ntaps)
	for i in range(0,Ntaps):
		if(i == (Ntaps-1)/2):
			h[i] = norm_pass
		else:
			h[i] =norm_pass* math.sin(math.pi * t * (i - (Ntaps-1)/2)) / (math.pi * t * (i - (Ntaps-1)/2))
		h[i] = h[i] * math.cos(i*math.pi*norm_center)* math.sin(i*math.pi/Ntaps)*math.sin(i*math.pi/Ntaps)

	return h


if __name__ == "__main__":

	norm_cutoff = [Fb/(Fs/2), Fe/(Fs/2)]
	h_target = signal.firwin(Ntaps, norm_cutoff, pass_zero=False, window='hann')
	h_test = banpass(Fb,Fe,Fs,Ntaps)

	w1, h1 = signal.freqz(h_target)
	w2, h2 = signal.freqz(h_test)

	fig, (ax0, ax1) = plt.subplots(nrows=2)
	ax0.set_title('Digital filter frequency response')
	ax0.plot(w1, 20 * np.log10(abs(h1)), 'b')

	ax1.set_title('my filter frequency response')
	ax1.plot(w2, 20 * np.log10(abs(h2)), 'b')

	err = max(abs(h1-h2))
	print(err)
	plt.show()
