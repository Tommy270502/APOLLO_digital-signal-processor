/*
 * LowPassFilter.h
 *
 *  Created on: Feb 3, 2023
 *      Author: Perri
 */

#ifndef INC_LOWPASSFILTER_H_
#define INC_LOWPASSFILTER_H_

typedef struct {

	float alpha[2];
	float out[2];

}LowPassFilter;

void init_LowPassFilter(LowPassFilter *handle, float cutoffFreq, float sampletimeS);
void update_LowPassFilter(LowPassFilter *handle, float input);

#endif /* INC_LOWPASSFILTER_H_ */
