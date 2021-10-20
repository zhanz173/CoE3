#include "estimatePSD.h"
#include "fourier.h"
#include "dy4.h"
using namespace std;


inline float abs_square(std::complex<float>& a) {
    return a.real() * a.real() + a.imag() * a.imag();
}



void estimatePSD(const std::vector<float>& samples, std::vector<float>& freq, std::vector<float>& psd_est, float Fs) {
    int freq_bins = samples.size();
    const float df = static_cast<float>(Fs) / freq_bins;
    int no_segments = static_cast<int> (floor(static_cast<float>(samples.size()) / freq_bins));
    std::vector<float> hann, psd_list;
    psd_est.resize(freq_bins >> 1);
    psd_list.resize(samples.size()>>1);

//for plotting the PSD on the Y axis (only positive freq)
    freq.resize(static_cast<size_t> (ceil(Fs / (2 * df))));
    for (auto i = 0; i < freq.size(); ++i)  freq[i] = i * df;

//hann windows
    hann.resize(freq_bins);
    for (auto i = 0; i < hann.size(); ++i)  hann[i] = sin(i * PI / freq_bins) * sin(i * PI / freq_bins);


//iterate through all the segments
    std::vector<std::complex<float>> Xf;
    int current_index = 0;
    int write_index = 0;
    for (int i = 0; i < no_segments; ++i) {
        current_index += freq_bins;
        auto tmp_data = vector<float>(samples.begin(),samples.begin()+current_index);
        for (int j = 0; j < no_segments; ++j) tmp_data[j] *= hann[j];
        fourier::DFT(tmp_data,Xf);

//translate to the decibel (dB) scale        
        for(int k = 0; k<freq_bins/2;++k)
            psd_list[k+write_index] = 2 * 10 * log10(4.0 / (Fs * freq_bins) * abs_square(Xf[k]));
        write_index += freq_bins/2;
    }

//compute the estimate to be returned by the function through averaging
    for (auto i = 0; i < psd_est.size(); ++i)
    {
        for (int j = 0; j < no_segments; ++j)
             psd_est[i] += psd_list[i + j * freq_bins/2];
        psd_est[i] = psd_est[i] / no_segments;

    }
}
