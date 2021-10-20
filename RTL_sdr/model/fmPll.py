#
# Comp Eng 3DY4 (Computer Systems Integration Project)
#
# Copyright by Nicola Nicolici
# Department of Electrical and Computer Engineering
# McMaster University
# Ontario, Canada
#

import numpy as np
import math

def fmPll(pllIn, freq, Fs, state,\
		ncoScale = 1.0, phaseAdjust = 0.0, normBandwidth = 0.01):

	"""
	pllIn 	 		array of floats
					input signal to the PLL (assume known frequency)

	freq 			float
					reference frequency to which the PLL locks

	Fs  			float
					sampling rate for the input/output signals

	ncoScale		float
					frequency scale factor for the NCO output

	phaseAdjust		float
					phase adjust to be added to the NCO only

	normBandwidth	float
					normalized bandwidth for the loop filter
					(relative to the sampling rate)

	state 			to be added

	"""

	# scale factors for proportional/state[0] terms
	# these scale factors were derived assuming the following:
	# damping factor of 0.707 (1 over square root of 2)
	# there is no oscillator gain and no phase detector gain
	Cp = 2.666
	Ci = 3.555

	# gain for the proportional term
	Kp = (normBandwidth)*Cp
	# gain for the state[0] term
	Ki = (normBandwidth*normBandwidth)*Ci

	# output array for the NCO
	ncoOut = np.empty(len(pllIn)+1)
	ncoOut[0] = state[4]
	# initialize internal state
	'''
		state[0] = 0.0
		state[1] = 0.0
		state[2] = 1.0
		state[3] = 0.0
		state[4] = 1.0
		state[5] = 0
	'''
	# note: state saving will be needed for block processing

	for k in range(len(pllIn)):

		# phase detector
		errorI = pllIn[k] * (+state[2])  # complex conjugate of the
		errorQ = pllIn[k] * (-state[3])  # feedback complex exponential

		# four-quadrant arctangent discriminator for phase error detection
		errorD = math.atan2(errorQ, errorI)

		# loop filter
		state[0] = state[0] + Ki*errorD

		# update phase estimate
		state[1] = state[1] + Kp*errorD + state[0]

		# internal oscillator
		trigArg = 2*math.pi*(freq/Fs)*(state[5]+k+1) + state[1]
		state[2] = math.cos(trigArg)
		state[3] = math.sin(trigArg)
		ncoOut[k+1] = math.cos(trigArg*ncoScale + phaseAdjust)

	# for stereo only the in-phase NCO component should be returned
	# for block processing you should also return the state


	state[4] = ncoOut[-1]
	return ncoOut[:-1]
	# for RDS add also the quadrature NCO component to the output

if __name__ == "__main__":

	pass
