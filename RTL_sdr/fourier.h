/*
Comp Eng 3DY4 (Computer Systems Integration Project)

Department of Electrical and Computer Engineering
McMaster University
Ontario, Canada
*/

#ifndef DY4_FOURIER_H
#define DY4_FOURIER_H

// add headers as needed
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#define FFTYPE float
// declaration of a function prototypes
namespace fourier
{
	void DFT(const std::vector<float> &,
		std::vector<std::complex<float>> &);

	// you should add your own IDFT
	// time-permitting you can build your own function for FFT

	void computeVectorMagnitude(const std::vector<std::complex<float>> &,
		std::vector<float> &);

	void FFT(std::vector<std::complex<FFTYPE>>&, int=0);

	void IFFT(std::vector<std::complex<FFTYPE>>&, int=0);

	void convolve(std::vector<std::complex<FFTYPE>>& __restrict , const std::vector<std::complex<FFTYPE>>& __restrict , int);
	// provide the prototype to estimate PSD
	// ...
}
#endif // DY4_FOURIER_H
