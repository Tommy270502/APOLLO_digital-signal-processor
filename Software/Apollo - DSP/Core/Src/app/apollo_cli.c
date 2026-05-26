#include "app/apollo_cli.h"

#include <stdio.h>
#include <string.h>
#include "app/apollo_config.h"

static void init_command(apollo_cli_command_t *command) {
	command->type = APOLLO_CLI_COMMAND_NONE;
	dsp_filter_default_config(&command->filter_config);
	command->filter_config.sample_period_s = APOLLO_DEFAULT_SAMPLE_PERIOD_S;
	command->filter_config.cutoff_hz = APOLLO_DEFAULT_FILTER_CUTOFF_HZ;
	command->channel = 0U;
	command->demo_mode = APOLLO_DEMO_OFF;
	command->telemetry_enabled = 1U;
	command->cal_gain = 1.0f;
	command->cal_offset = 0.0f;
}

static apollo_status_t parse_filter(const char *line, apollo_cli_command_t *command) {
	float value = 0.0f;
	unsigned int window = 0U;

	command->type = APOLLO_CLI_COMMAND_FILTER;

	if (strcmp(line, "filter bypass") == 0) {
		command->filter_config.mode = DSP_FILTER_MODE_BYPASS;
		return APOLLO_STATUS_OK;
	}

	if (sscanf(line, "filter lowpass %f", &value) == 1) {
		command->filter_config.mode = DSP_FILTER_MODE_LOWPASS;
		command->filter_config.cutoff_hz = value;
		return APOLLO_STATUS_OK;
	}

	if (sscanf(line, "filter highpass %f", &value) == 1) {
		command->filter_config.mode = DSP_FILTER_MODE_HIGHPASS;
		command->filter_config.cutoff_hz = value;
		return APOLLO_STATUS_OK;
	}

	if (sscanf(line, "filter ema %f", &value) == 1) {
		command->filter_config.mode = DSP_FILTER_MODE_EMA;
		command->filter_config.alpha = value;
		return APOLLO_STATUS_OK;
	}

	if (sscanf(line, "filter average %u", &window) == 1) {
		command->filter_config.mode = DSP_FILTER_MODE_MOVING_AVERAGE;
		command->filter_config.window_size = (uint8_t)window;
		return APOLLO_STATUS_OK;
	}

	if (sscanf(line, "filter median %u", &window) == 1) {
		command->filter_config.mode = DSP_FILTER_MODE_MEDIAN;
		command->filter_config.window_size = (uint8_t)window;
		return APOLLO_STATUS_OK;
	}

	return APOLLO_STATUS_INVALID_ARG;
}

static apollo_status_t parse_demo(const char *line, apollo_cli_command_t *command) {
	command->type = APOLLO_CLI_COMMAND_DEMO;

	if (strcmp(line, "demo off") == 0) {
		command->demo_mode = APOLLO_DEMO_OFF;
	} else if (strcmp(line, "demo sine") == 0) {
		command->demo_mode = APOLLO_DEMO_SINE;
	} else if (strcmp(line, "demo step") == 0) {
		command->demo_mode = APOLLO_DEMO_STEP;
	} else if (strcmp(line, "demo impulse") == 0) {
		command->demo_mode = APOLLO_DEMO_IMPULSE;
	} else {
		return APOLLO_STATUS_INVALID_ARG;
	}

	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_cli_parse(const char *line, apollo_cli_command_t *command) {
	unsigned int channel = 0U;

	if (line == NULL || command == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	init_command(command);

	if (line[0] == '\0') {
		return APOLLO_STATUS_OK;
	}

	if (strcmp(line, "help") == 0) {
		command->type = APOLLO_CLI_COMMAND_HELP;
		return APOLLO_STATUS_OK;
	}

	if (strcmp(line, "status") == 0) {
		command->type = APOLLO_CLI_COMMAND_STATUS;
		return APOLLO_STATUS_OK;
	}

	if (strncmp(line, "filter ", 7U) == 0) {
		return parse_filter(line, command);
	}

	if (sscanf(line, "input %u", &channel) == 1) {
		if (channel > 1U) {
			return APOLLO_STATUS_INVALID_ARG;
		}
		command->type = APOLLO_CLI_COMMAND_INPUT;
		command->channel = (uint8_t)channel;
		return APOLLO_STATUS_OK;
	}

	if (strncmp(line, "demo ", 5U) == 0) {
		return parse_demo(line, command);
	}

	if (strcmp(line, "telemetry on") == 0) {
		command->type = APOLLO_CLI_COMMAND_TELEMETRY;
		command->telemetry_enabled = 1U;
		return APOLLO_STATUS_OK;
	}

	if (strcmp(line, "telemetry off") == 0) {
		command->type = APOLLO_CLI_COMMAND_TELEMETRY;
		command->telemetry_enabled = 0U;
		return APOLLO_STATUS_OK;
	}

	if (strcmp(line, "calibrate clear") == 0) {
		command->type = APOLLO_CLI_COMMAND_CALIBRATE_CLEAR;
		return APOLLO_STATUS_OK;
	}

	if (sscanf(line, "calibrate adc %f %f", &command->cal_gain, &command->cal_offset) == 2) {
		command->type = APOLLO_CLI_COMMAND_CALIBRATE_ADC;
		return APOLLO_STATUS_OK;
	}

	if (sscanf(line, "calibrate dac %f %f", &command->cal_gain, &command->cal_offset) == 2) {
		command->type = APOLLO_CLI_COMMAND_CALIBRATE_DAC;
		return APOLLO_STATUS_OK;
	}

	return APOLLO_STATUS_INVALID_ARG;
}
