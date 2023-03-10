/*
 * LowPassFilter.c
 *
 *  Created on: Feb 3, 2023
 *      Author: Perri
 */
#include "LowPassFilter.h"
#include  <stdint.h>
#include <stdio.h>

void init_LowPassFilter(LowPassFilter *handle, float cutoffFreq, float sampletimeS) {

	float RC = 1.00f / (6.28318531f * cutoffFreq);

	//compute alpha coefficient
	handle->alpha[0] = sampletimeS / (sampletimeS + RC);
	//compute the inverse value of alpha coefficient
	handle->alpha[1] = RC / (sampletimeS + RC);

	//clear output buffer
	handle->out[0] = 0.00f;
	handle->out[1] = 0.00f;
}

void update_LowPassFilter(LowPassFilter *handle, float input) {

	//shift output sample
	handle->out[1] = handle->out[0];

	//compute new output sample
	handle->out[0] = handle->alpha[0] * input + handle->alpha[1] * handle->out[1];
}
