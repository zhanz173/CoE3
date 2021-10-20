#ifndef PLL_H
#define PLL_H
#include <vector>
#include <math.h>
#define ONEQTR_PI  PI / 4.0;
#define THRQTR_PI  3.0 * PI / 4.0;

struct STATE {
	float integrator = 0.0;
	float phaseEst = 0.0;
	float feedbackI = 1.0;
	float feedbackQ = 0.0;
	float ncoOut_zero = 1.0;
	float trigOffset = 0.0;
};

void fmPll(std::vector<float>&, std::vector<float>&, float, float, float, float, float, STATE*);
#endif