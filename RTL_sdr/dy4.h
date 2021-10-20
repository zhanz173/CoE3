/*
Comp Eng 3DY4 (Computer Systems Integration Project)

Department of Electrical and Computer Engineering
McMaster University
Ontario, Canada
*/

#ifndef DY4_DY4_H
#define DY4_DY4_H

// some general and reusable stuff
// our beloved PI constant
#define PI 3.14159265358979323846
#define N_TAPS0 151
#define N_TAPS1 151*24
#define rf_Fs0 2.4e6f
#define rf_Fs1 2.5e6f
#define rf_Fc 100e3f
#define rf_decim 10
#define audio_Fc 16e3
// although we use DFT (no FFT ... yet), the number of points for a
// Fourier transform is defined as NFFT (same as matplotlib)
#define NFFT 512

#endif // DY4_DY4_H
