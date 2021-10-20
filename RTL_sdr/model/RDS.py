import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal
import numpy as np
import math
from bandpass import banpass
from Mono_mode1 import expand

# use "custom" fmDemodArctan
from fmSupportLib import fast_Demod, fmDemodArctan
from fmPll import fmPll
from fmRRC import impulseResponseRootRaisedCosine

rf_Fs = 2.4e6
rf_Fc = 100e3
rf_taps = 151
rf_decim = 10

Ntaps = 101
audio_Fs = 48e3
audio_Fc = 16e3
audio_taps = 151
audio_decim = 5

#Channel Extraction
Fb = 54e3
Fe = 60e3
Fs = 240e3

#Carrier Recovery
Fb2 = 113.5e3
Fe2 = 114.5e3
Fs2 = 240e3

#Squaring nonlinearity
def squaring(rds_channel):
    data_out = np.zeros(len(rds_channel))
    for i in range(len(rds_channel)):
        data_out[i]= rds_channel[i]*rds_channel[i]
    return data_out

def mixer(carrier, channel):
	data_out = np.zeros(len(carrier))
	for i in range(len(carrier)):
		data_out[i] = carrier[i]*channel[i]
	return data_out

if __name__ == "__main__":

    in_fname = "/home/pi/Desktop/project-group22-wednesday/data/mode1_samples3_u8.raw"
    raw_data = np.fromfile(in_fname, dtype='uint8')
    iq_data = (raw_data - 128.0)/128.0
    print("Read raw RF data from \"" + in_fname + "\" in unsigned 8-bit format")
    state = [0.0, 0.0, 1.0, 0.0, 1.0, 0.0]

    #Extract coefficients
    rf_coeff = signal.firwin(rf_taps, \
		rf_Fc/(rf_Fs/2), window=('hann'))

    audio_coeff = signal.firwin(audio_taps, \
		audio_Fc/((rf_Fs/rf_decim)/2), window=('hann'))

    BPF_54K = banpass(Fb,Fe,Fs,Ntaps)
    BPF_113K = banpass(Fb2,Fe2,Fs2,Ntaps)

    fig, (ax0, ax1, ax2, ax3) = plt.subplots(nrows=4)
    fig.subplots_adjust(hspace = 1.0)

    block_size = 1024 * rf_decim * audio_decim * 2
    block_count = 0

    state_i_lpf_100k = np.zeros(rf_taps-1)
    state_q_lpf_100k = np.zeros(rf_taps-1)
    state_lpf_16k = np.zeros(audio_taps-1)
    state_bpf_54K = np.zeros(Ntaps-1)
    state_bpf_113K = np.zeros(Ntaps-1)
    state_lpf_3k = np.zeros(rf_taps-1)
    state_phase = 0

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

        #FM demod
        fm_demod, state_phase = fast_Demod(i_ds, q_ds, state_phase)

        #audio_filt, state_lpf_16k = signal.lfilter(audio_coeff, 1.0, \
		#		fm_demod, zi=state_lpf_16k)

        #RDS channel extraction
        rds_channel, state_bpf_54K = signal.lfilter(BPF_54K, 1.0, fm_demod, zi = state_bpf_54K)

        #RDS carrier recovery
        squared = squaring(rds_channel)
        rds_carrier, state_bpf_113K = signal.lfilter(BPF_113K, 1.0, squared, zi = state_bpf_113K)
        recovered_rds = fmPll(rds_carrier, 114e3, rf_Fs/10, state, 0.5)

        RDS_front = mixer(recovered_rds, rds_channel)

        #RDS demodulation
        RDS_coeff = signal.firwin(rf_taps, \
			3e3/(rf_Fs/2), window=('hann'))
        RDSfront_filt, state_lpf_3k = signal.lfilter(RDS_coeff, 1.0, RDS_front, zi = state_lpf_3k )
        RDS_resampled=expand(RDSfront_filt,24)
        RDS_RRC = impulseResponseRootRaisedCosine(Fs,Ntaps)

        # to save runtime select the range of blocks to log data
        # this includes both saving binary files as well plotting PSD
        # below we assume we want to plot for graphs for blocks 10 and 11
        if block_count >= 0 and block_count < 1:
            #PSD after FM demodulation
            ax0.clear()
            ax0.psd(fm_demod, NFFT=512, Fs=(rf_Fs/rf_decim)/1e3)
            ax0.set_ylabel('PSD (dB/Hz)')
            ax0.set_xlabel('Freq (kHz)')
            ax0.set_title('Demodulated FM (block ' + str(block_count) + ')')
            # output binary file name (where samples are written from Python)
            fm_demod_fname = "../data/fm_demod_" + str(block_count) + ".bin"
            # create binary file where each sample is a 32-bit float
            fm_demod.astype('float32').tofile(fm_demod_fname)

            # PSD after extracting mono audio
            ax1.clear()
            ax1.psd(recovered_rds, NFFT=512, Fs=(rf_Fs/rf_decim)/1e3)
            ax1.set_ylabel('PSD (dB/Hz)')
            ax1.set_xlabel('Freq (kHz)')
            ax1.set_title('Recovered RDS lol')

            ax2.clear()
            ax2.psd(RDS_front, NFFT=512, Fs=(rf_Fs/rf_decim)/1e3)
            ax2.set_ylabel('PSD (dB/Hz)')
            ax2.set_xlabel('Freq (kHz)')
            ax2.set_title('Recovered_RDS mixed with RDS channel')


            # ax3.clear()
            # time = np.arange(0,65,.25)
            # ax3.plot(time,recovered_rds,'bs')
            # ax3.set_xlabel('Time')
            # ax3.set_ylabel('Amplitude')
            # ax3.set_title("Recovered_RDS Time Domain")
            # ax3.show()

            # save figure to file
            fig.savefig("../data/Recovered_RDS" + str(block_count) + ".png")

        block_count += 1
    #wavfile.write("../data/fmAudio_RDS.wav", int(48e3), np.int16((audio/2)*32767))
    print('Finished processing the raw I/Q samples')
