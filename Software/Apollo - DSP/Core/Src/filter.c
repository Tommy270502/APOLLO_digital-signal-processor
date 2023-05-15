/*
 * filter.c
 *
 *  Created on: Feb 3, 2023
 *      Author: Perri
 */
#include "filter.h"
#include  <stdint.h>
#include <stdio.h>

void init_LowPassFilter(LowPassFilter *handle, float cutoffFreq, float sampletimeS) {

	float RC = 1.00f / (6.28318531f * cutoffFreq);

	//compute alpha coefficient

	//make sure RC is minimum 2x sampletimeS (improves frequency response and performance)
	float twoX = RC / sampletimeS;

	if(twoX <= 2) {
		RC = RC * (2 / twoX);
	}

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


void init_HighPassFilter(HighPassFilter *handle, float cutoffFreq, float sampletimeS) {

	float RC = 1.00f / (6.28318531f * cutoffFreq);

	//compute alpha coefficient

	//make sure RC is minimum 2x sampletimeS (improves frequency response and performance)
	float twoX = RC / sampletimeS;

	if(twoX <= 2) {
		RC = RC * (2 / twoX);
	}

	handle->alpha[0] = sampletimeS / (sampletimeS + RC);
	//compute the inverse value of alpha coefficient
	handle->alpha[1] = RC / (sampletimeS + RC);

	//clear output buffer
	handle->out[0] = 0.00f;
	handle->out[1] = 0.00f;
}

void update_HighPassFilter(HighPassFilter *handle, float input) {

	//shift output sample
	handle->input[0] = input;
	handle->out[1] = handle->out[0];

	//compute new output sample
	handle->out[0] = handle->alpha[0] * input + handle->alpha[1] * handle->out[1];
	handle->out[0] = handle->alpha[0] * (handle->out[1] + handle->input[0] - handle->input[1]);
	handle->input[1] = handle->input[0];
}
