#include"dy4.h"
#include "PLL.h"

using namespace std;

float atan2_approximation1(float y, float x)
{
	//http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
	//Volkan SALMA
	float r, angle;
	float abs_y = fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if (x < 0.0f)
	{
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	}
	else
	{
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;
	if (y < 0.0f)
		return(-angle);     // negate if in quad III or IV
	else
		return(angle);
}


void fmPll(vector<float>& pllIn, vector<float>& ncoOut, float freq, float Fs, float ncoScale, float phaseAdjust, float normBandwidth, STATE* state) {
	float Cp = 2.666;
	float Ci = 3.555;
	float Kp = (normBandwidth)*Cp;
	float Ki = (normBandwidth * normBandwidth) * Ci;

	//auto FastArcTan = [](float x) {	return PI * x - x * (fabs(x) - 1) * (0.2447 + 0.0663 * fabs(x));	};
	float errorI, errorQ, errorD, trigArg;
	ncoOut[0] = state->ncoOut_zero;
	for (int k = 0; k < ncoOut.size(); k++) {
		errorI = pllIn[k] * state->feedbackI;
		errorQ = -(pllIn[k] * state->feedbackI);
		errorD = atan2_approximation1(errorQ, errorI);

		state->integrator = state->integrator + Ki * errorD;
		state->phaseEst = state->phaseEst + Kp * errorD + state->integrator;

		trigArg = 2 * PI * (freq / Fs) * (state->trigOffset + k + 1) + state->phaseEst;

		state->feedbackI = cosf(trigArg);
		state->feedbackQ = sinf(trigArg);
		if(k == ncoOut.size() -1)	state->ncoOut_zero = cosf(trigArg * ncoScale + phaseAdjust);
		else   ncoOut[k + 1] = cosf(trigArg * ncoScale + phaseAdjust);
	}
}