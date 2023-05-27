/*
 * filter.h
 *
 *  Created on: Feb 3, 2023
 *      Author: Perri
 */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_

typedef struct {

	double alpha[2];
	double input[2];
	double out[2];

}HighPassFilter;

void init_HighPassFilter(HighPassFilter *handle, double cutoffFreq, double sampletimeS);
void update_HighPassFilter(HighPassFilter *handle, double input);

typedef struct {

	double alpha[2];
	double out[2];

}LowPassFilter;

void init_LowPassFilter(LowPassFilter *handle, double cutoffFreq, double sampletimeS);
void update_LowPassFilter(LowPassFilter *handle, double input);


#endif
