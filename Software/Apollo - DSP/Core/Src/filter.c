/*
 * filter.c
 *
 *  Created on: Feb 3, 2023
 *      Author: Perri
 */
#include "filter.h"

static double filter_rc(double cutoffFreq, double sampletimeS) {
	double RC = 1.0 / (6.28318531 * cutoffFreq);
	double min_rc = sampletimeS * 2.0;

	if (RC < min_rc) {
		RC = min_rc;
	}

	return RC;
}

void init_HighPassFilter(HighPassFilter *handle, double cutoffFreq, double sampletimeS) {
	double RC = filter_rc(cutoffFreq, sampletimeS);

	handle->alpha[0] = sampletimeS / (sampletimeS + RC);
	handle->alpha[1] = RC / (sampletimeS + RC);

	handle->input[0] = 0.00f;
	handle->input[1] = 0.00f;
	handle->out[0] = 0.00f;
	handle->out[1] = 0.00f;
}

void update_HighPassFilter(HighPassFilter *handle, double input) {
	handle->input[0] = input;
	handle->out[1] = handle->out[0];

	handle->out[0] = handle->alpha[1] * (handle->out[1] + handle->input[0] - handle->input[1]);
	handle->input[1] = handle->input[0];
}

void init_LowPassFilter(LowPassFilter *handle, double cutoffFreq, double sampletimeS) {
	double RC = filter_rc(cutoffFreq, sampletimeS);

	handle->alpha[0] = sampletimeS / (sampletimeS + RC);
	handle->alpha[1] = RC / (sampletimeS + RC);

	handle->out[0] = 0.00f;
	handle->out[1] = 0.00f;
}

void update_LowPassFilter(LowPassFilter *handle, double input) {
	handle->out[1] = handle->out[0];
	handle->out[0] = handle->alpha[0] * input + handle->alpha[1] * handle->out[1];
}
