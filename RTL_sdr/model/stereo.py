import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal
import numpy as np
import math
from bandpass import banpass

# use "custom" fmDemodArctan
from fmSupportLib import fast_Demod, fmDemodArctan
from fmPll import fmPll

rf_Fs = 2.4e6
rf_Fc = 100e3
rf_taps = 151
rf_decim = 10

Ntaps = 101
audio_Fs = 48e3
audio_Fc = 16e3
audio_taps = 151
audio_decim = 5

Fb = 18.5e3
Fe = 19.5e3
Fs = 240e3

Fb2 = 22e3
Fe2 = 54e3
Fs2 = 240e3



def mixer(carrier, channel):
	data_out = np.zeros(len(carrier))
	for i in range(len(carrier)):
		data_out[i] = carrier[i]*channel[i]
	return data_out





if __name__ == "__main__":
	in_fname = "D:/3DY4/project-group22-wednesday/data/mode0_u8.raw"
	raw_data = np.fromfile(in_fname, dtype='uint8')
	iq_data = (raw_data - 128.0)/128.0
	print("Read raw RF data from \"" + in_fname + "\" in unsigned 8-bit format")
	state = [0.0, 0.0, 1.0, 0.0, 1.0, 0.0]

	rf_coeff = signal.firwin(rf_taps, \
		rf_Fc/(rf_Fs/2), window=('hann'))

	audio_coeff = signal.firwin(audio_taps, \
		audio_Fc/((rf_Fs/rf_decim)/2), window=('hann'))
	
	norm_cutoff = [Fb/(Fs/2), Fe/(Fs/2)]
	BPF_19k = banpass(Fb,Fe,Fs,Ntaps)

	norm_cutoff = [Fb2/(Fs/2), Fe2/(Fs/2)]
	BPF_22K = banpass(Fb2,Fe2,Fs2,Ntaps)

	block_size = 480 * rf_decim * audio_decim * 2
	block_count = 0

	state_i_lpf_100k = np.zeros(rf_taps-1)
	state_q_lpf_100k = np.zeros(rf_taps-1)
	state_lpf_16k = np.zeros(audio_taps-1)
	state_bpf_19k = np.zeros(Ntaps-1)
	state_bpf_22k = np.zeros(Ntaps-1)
	state_phase = 0

	audio_L = np.array([])
	audio_R = np.array([])

	ncoOut = np.array([])

	while (block_count+1)*block_size < len(iq_data):
		print('Processing block ' + str(block_count))

		# filter to extract the FM channel (I samples are even, Q samples are odd)
		i_filt, state_i_lpf_100k = signal.lfilter(rf_coeff, 1.0, \
				iq_data[(block_count)*block_size:(block_count+1)*block_size:2],
				zi=state_i_lpf_100k)
		q_filt, state_q_lpf_100k = signal.lfilter(rf_coeff, 1.0, \
				iq_data[(block_count)*block_size+1:(block_count+1)*block_size:2],
				zi=state_q_lpf_100k)

		i_ds = i_filt[::rf_decim]
		q_ds = q_filt[::rf_decim]

		fm_demod, state_phase = fast_Demod(i_ds, q_ds, state_phase)

		audio_filt, state_lpf_16k = signal.lfilter(audio_coeff, 1.0, \
				fm_demod, zi=state_lpf_16k)

		# downsample audio data
		audio_block1 = audio_filt[::audio_decim]
		#
		#
		#stereo path
		stereo_carrier,state_bpf_19k = signal.lfilter(BPF_19k,1.0, fm_demod, zi=state_bpf_19k)
		ncoOut_prev = ncoOut
		ncoOut = fmPll(stereo_carrier,19e3,rf_Fs/10,state, 2.0)
		stereo_channel,state_bpf_22k = signal.lfilter(BPF_22K,1.0, fm_demod, zi=state_bpf_22k)

		stereo_front = mixer(ncoOut,stereo_channel)

		#reperat mono path
		audio_filt2, state_lpf_16k = signal.lfilter(audio_coeff, 1.0, \
				stereo_front, zi=state_lpf_16k)

		audio_block2 = audio_filt2[::audio_decim]

		audio_L_block = audio_block1 + audio_block2
		audio_R_block = audio_block1 - audio_block2

		audio_L = np.concatenate((audio_L, audio_L_block))
		audio_R = np.concatenate((audio_R, audio_R_block))
		# to save runtime select the range of blocks to log data
		# this includes both saving binary files as well plotting PSD
		# below we assume we want to plot for graphs for blocks 10 and 11
		if block_count >= 10 and block_count < 11:
			# PSD after FM demodulation
			test = np.concatenate((ncoOut_prev[:-100], ncoOut[:100]))
			plt.plot(test[:200],'b')
			plt.plot(stereo_carrier[:200],'g')

			plt.show()


		block_count += 1
	print('Finished processing the raw I/Q samples')