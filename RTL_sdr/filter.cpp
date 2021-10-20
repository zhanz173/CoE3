/*
Comp Eng 3DY4 (Computer Systems Integration Project)

Department of Electrical and Computer Engineering
McMaster University
Ontario, Canada
*/

#include "dy4.h"
#include "filter.h"



void impulseResponseLPF(float Fs, float Fc, int num_taps, std::vector<float> &h)
{
	float tmp = 0;
	h.reserve(num_taps);
	float norm_cutoff = Fc / (Fs / 2);

	for (int i = 0; i < num_taps; i++) {
		if (i == static_cast<int>((num_taps - 1) / 2))
			tmp = norm_cutoff;
		else {
			tmp = norm_cutoff * std::sin((PI * norm_cutoff * (i - (num_taps - 1) / 2.0))) / (PI * norm_cutoff * (i - (num_taps - 1) / 2.0));
		}
		h.push_back(tmp * std::sin(i * PI / num_taps) * std::sin(i * PI / num_taps));
	}
}

//assume block size larger than filter size



void impulseResponseBPF(float Fb, float Fe, float Fs, int num_taps, std::vector<float>& h) {
	float norm_center = (Fb + Fe) / Fs;
	float norm_pass = (Fe - Fb) / (Fs / 2);
	float half_norm_pass = norm_pass / 2.0;
	h.reserve(num_taps);
	
	float tmp = 0;
	for (int i = 0; i < num_taps; i++) {
		if (i == static_cast<int>((num_taps - 1) / 2))
			tmp = norm_pass;
		else {
			tmp = norm_pass * std::sin((PI * half_norm_pass * (i - (num_taps - 1) / 2.0))) / (PI * half_norm_pass * (i - (num_taps - 1) / 2.0));
		}
		tmp *= std::sin(i * PI / num_taps) * std::sin(i * PI / num_taps) * std::cos(i * PI * norm_center);
		h.push_back(tmp);
	}
}

void impulseResponseRootRaisedCosine(float Fs, int N_taps, std::vector<float>& h) {
	float T_symbol = 1 / 2375.0;
	float beta = 0.90;
	h.resize(N_taps);

	for (int k = 0; k < N_taps; k++) {
		float t = static_cast<float>(((k - N_taps / 2.0))) / Fs;

		if (t == 0.0)
			h[k] = 1.0 + beta * ((4.0 / PI) - 1);
		else if (t == -T_symbol / (4 * beta) || t == T_symbol / (4 * beta))
			h[k] = (beta / sqrt(2)) * (((1 + 2 / PI) * (sin(PI / (4 * beta)))) + ((1 - 2 / PI) * (cos(PI / (4 * beta)))));
		else
			h[k] = (sin(PI * t * (1 - beta) / T_symbol) + \
				4 * beta * (t / T_symbol) * cos(PI * t * (1 + beta) / T_symbol)) / \
			(PI * t * (1 - (4 * beta * t / T_symbol) * (4 * beta * t / T_symbol)) / T_symbol);
	}
}

