/*
 * filter.h
 *
 *  Created on: Feb 3, 2023
 *      Author: Perri
 */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_

typedef struct {

	float alpha[2];
	float input[2];
	float out[2];

}HighPassFilter;

void init_HighPassFilter(HighPassFilter *handle, float cutoffFreq, float sampletimeS);
void update_HighPassFilter(HighPassFilter *handle, float input);

typedef struct {

	float alpha[2];
	float out[2];

}LowPassFilter;

void init_LowPassFilter(LowPassFilter *handle, float cutoffFreq, float sampletimeS);
void update_LowPassFilter(LowPassFilter *handle, float input);


#endif
