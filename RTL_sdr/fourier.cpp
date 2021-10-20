/*
Comp Eng 3DY4 (Computer Systems Integration Project)

Department of Electrical and Computer Engineering
McMaster University
Ontario, Canada
*/

// source code for Fourier-family of functions
#include "dy4.h"
#include "fourier.h"

namespace fourier{
	// just DFT function (no FFT yet)
	void DFT(const std::vector<float> &x, std::vector<std::complex<float>> &Xf) {
		Xf.resize(x.size(), static_cast<std::complex<float>>(0, 0));
		for (auto m = 0; m < Xf.size(); m++) {
			for (auto k = 0; k < x.size(); k++) {
					std::complex<float> expval(0, -2*PI*(k*m) / x.size());
					Xf[m] += x[k] * std::exp(expval);
			}
		}
	}

	// function to compute the magnitude values in a complex vector
	void computeVectorMagnitude(const std::vector<std::complex<float>> &Xf, std::vector<float> &Xmag)
	{
		// only the positive frequencies
		Xmag.resize(Xf.size(), static_cast<float>(0));
	for (auto i = 0; i < Xf.size(); i++) {
		Xmag[i] = std::abs(Xf[i])/Xf.size();
	}
	}

	template<typename T = uint16_t, size_t N=NFFT>
	struct index{
		constexpr index(): INDEX() {bit_reveral_permutation();}
		constexpr void bit_reveral_permutation();

		T INDEX[N];
	};

	template<typename T, size_t N>
	constexpr void index<T,N>::bit_reveral_permutation(){
		static_assert((int)(NFFT&(NFFT-1)) == 0);
		auto k = 1, m = NFFT>>1;
		INDEX[0] = 0;
		for(auto i = 1; i < NFFT; i<<=1){
			for(auto j=0; j<k; j++){
				INDEX[j+i] = INDEX[j] + m;
			}
			k <<=1;
			m >>= 1;
		}
	}   

	index<> constexpr ID;
	auto INDEX = ID.INDEX;

    void FFT(std::vector<std::complex<FFTYPE>>& data_out, int offset)
	{
		for(auto i = 0; i < NFFT; ++i){
			if(i < INDEX[i]) std::swap(data_out[offset+i],data_out[offset+INDEX[i]]);
		}

		int block_size, step=1;
		while(step<NFFT){
			block_size = step<<1;
			std::complex<FFTYPE> P(0,-2*PI/block_size);      
			std::complex<FFTYPE> W = std::exp(P);        //W1
			std::complex<FFTYPE> W_tmp(1,0), temp;
			for(auto m=offset; m < offset+step; ++m){
				for(auto j=m; j<offset+NFFT; j+=block_size){
					temp = data_out[j];
					data_out[j] = data_out[j] + W_tmp*data_out[j+step];
					data_out[j+step] = temp - W_tmp*data_out[j+step];
				}
				W_tmp = W_tmp*W;
			}
			step = block_size;
		}

	}

	//The result is scaled up by N !!!

    void IFFT(std::vector<std::complex<FFTYPE>>& data_out, int offset)
	{
		for(auto i = 0; i < NFFT; ++i){
			if(i < INDEX[i]) std::swap(data_out[offset+i],data_out[offset+INDEX[i]]);
		}

		int block_size, step=1;
		while(step<NFFT){
			block_size = step<<1;
			std::complex<FFTYPE> P(0,2*PI/block_size);      
			std::complex<FFTYPE> W = std::exp(P);        //W1
			std::complex<FFTYPE> W_tmp(1,0), temp;
			for(auto m=offset; m < offset+step; ++m){
				for(auto j=m; j<offset+NFFT; j+=block_size){
					temp = data_out[j];
					data_out[j] = data_out[j] + W_tmp*data_out[j+step];
					data_out[j+step] = temp - W_tmp*data_out[j+step];
				}
				W_tmp = W_tmp*W;
			}
			step = block_size;
		}

	}

	void convolve(std::vector<std::complex<FFTYPE>>& __restrict data, const std::vector<std::complex<FFTYPE>>& __restrict h, int offset=0)
	{
		float scaling = 1.0/NFFT;
		for(auto i = 0; i < h.size(); ++i)
		{
			data[offset+i] = data[offset+i]*h[i]*scaling;
		}
	}
}