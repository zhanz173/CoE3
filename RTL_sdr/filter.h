/*
Comp Eng 3DY4 (Computer Systems Integration Project)

Department of Electrical and Computer Engineering
McMaster University
Ontario, Canada
*/

#ifndef DY4_FILTER_H
#define DY4_FILTER_H

// add headers as needed
#include <iostream>
#include <vector>
#include <math.h>
#include <climits>

// declaration of a function prototypes

void impulseResponseLPF(float, float, int, std::vector<float> &);
void impulseResponseBPF(float, float, float, int, std::vector<float>&);
void impulseResponseRootRaisedCosine(float, int, std::vector<float>&);
/// <summary>
/// circular convolution with gaps between each sample, the implementation based on overlap and add method with reuse of some data from previous block,
/// the convolution happens in-place without additional allocation however however
/// the input vector must be circular buffer type with proper lock mechanism
/// </summary>
/// <param name="x"> input data</param>
/// <param name="h"> filter coefficient</param>
/// <param name="data_out"> output data</param>
/// <param name="block_size"> number of samples</param>
/// <param name="downsmple"> downsample size</param>
/// <param name="mode"> 0/1</param>
/// <param name="x_start"> current offset in x</param>
/// <param name="y_start"> current offset in data_out</param>
/// <returns>return x offset of next block</returns>
template <typename  T>
int convolveFIR(const std::vector<float>& x, const std::vector<float>& h, std::vector<T>& data_out, const int x_block_size, const int downsample=1, const int mode=0, int x_start=0, int y_start=0, const int upsample = 24)
{
	int c = mode * 2 + (x_start == 0);
	float tmp = 0;
	switch (c) {
		//MODE 0 cases
	case 0:
	{
		for (int i = x_start; i < x_start + x_block_size; i += downsample) {
			for (int j = 0; j < h.size(); ++j) {
				tmp += h[j] * x[i - j];
			}
			if constexpr (std::is_same_v<T, short int>) {
				if (abs(tmp) < 1.0)	data_out[y_start++] = tmp * SHRT_MAX;
				else data_out[y_start++] = tmp > 0.0 ? SHRT_MAX : SHRT_MIN;
			}
			else data_out[y_start++] = (tmp);
			tmp = 0;;
		}
		break;
	}

	case 1:
	{
		for (int i = x_start; i < x_start + x_block_size; i += downsample) {
			for (int j = 0; j < h.size(); ++j) {
				if (i - j < 0)
					tmp += h[j] * x[i - j + x.size()];
				else
					tmp += h[j] * x[i - j];
			}
			if constexpr (std::is_same_v<T, short int>) {
				if (abs(tmp) < 1.0)	data_out[y_start++] = tmp * SHRT_MAX;
				else data_out[y_start++] = tmp > 0.0 ? SHRT_MAX : SHRT_MIN;
			}
			else data_out[y_start++] = (tmp);
			tmp = 0;
		}
		break;
	}
	//MODE 1 cases
	case 2:
	{
		int i, j, reminder, xindex;
		for (i = 0; i < x_block_size * upsample; i += downsample) {
			//phase match
			reminder = i % upsample;
			xindex = i / upsample + x_start;
			tmp = 0;
			for (j = reminder; j < h.size(); j += upsample) {
				tmp += h[j] * x[xindex--];
			}
			if constexpr (std::is_same_v<T, short int>) {
				if (abs(tmp) < 1.0)	data_out[y_start++] = tmp * SHRT_MAX;
				else data_out[y_start++] = tmp > 0.0 ? SHRT_MAX : SHRT_MIN;
			}
			else data_out[y_start++] = (tmp);
		}
		break;
	}

	case 3:
	{
		int i, j, reminder, xindex;
		for (i = 0; i < x_block_size * upsample; i += downsample) {
			//phase match
			reminder = i % upsample;
			xindex = i / upsample;
			tmp = 0;
			for (j = reminder; j < h.size(); j += upsample) {
				if (i - j < 0)
					tmp += h[j] * x[xindex + x.size()];
				else
					tmp += h[j] * x[xindex];
				xindex--;
			}
			if constexpr (std::is_same_v<T, short int>) {
				if (abs(tmp) < 1.0)	data_out[y_start++] = tmp * SHRT_MAX;
				else data_out[y_start++] = tmp > 0.0 ? SHRT_MAX : SHRT_MIN;
			}
			else data_out[y_start++] = (tmp);
		}
		break;
	}
	}
	//return the start of next block
	return (x_start + x_block_size) % x.size();
}
#endif // DY4_FILTER_H
