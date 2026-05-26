#include <stdio.h>
#include <string.h>

#include "app/apollo_cli.h"
#include "app/apollo_diagnostics.h"
#include "app/apollo_signal_chain.h"
#include "app/apollo_telemetry.h"
#include "dsp/dsp_filters.h"

static int failures;

static float abs_f(float value) {
	return (value < 0.0f) ? -value : value;
}

static void expect_true(int condition, const char *name) {
	if (!condition) {
		printf("FAIL: %s\n", name);
		failures++;
	}
}

static void expect_near(float actual, float expected, float tolerance, const char *name) {
	if (abs_f(actual - expected) > tolerance) {
		printf("FAIL: %s actual=%f expected=%f\n", name, (double)actual, (double)expected);
		failures++;
	}
}

static void test_lowpass_step(void) {
	dsp_filter_t filter;
	dsp_filter_config_t config;
	float output = 0.0f;
	float previous = 0.0f;

	dsp_filter_default_config(&config);
	config.mode = DSP_FILTER_MODE_LOWPASS;
	config.cutoff_hz = 10.0f;
	config.sample_period_s = 0.001f;

	expect_true(dsp_filter_init(&filter, &config) == APOLLO_STATUS_OK, "lowpass init");
	for (int i = 0; i < 20; i++) {
		expect_true(dsp_filter_update(&filter, 1000.0f, &output) == APOLLO_STATUS_OK, "lowpass update");
		expect_true(output >= previous, "lowpass monotonic step");
		previous = output;
	}

	expect_true(output > 0.0f && output < 1000.0f, "lowpass bounded step");
}

static void test_highpass_step_decay(void) {
	dsp_filter_t filter;
	dsp_filter_config_t config;
	float output = 0.0f;
	float first = 0.0f;

	dsp_filter_default_config(&config);
	config.mode = DSP_FILTER_MODE_HIGHPASS;
	config.cutoff_hz = 10.0f;
	config.sample_period_s = 0.001f;

	expect_true(dsp_filter_init(&filter, &config) == APOLLO_STATUS_OK, "highpass init");
	expect_true(dsp_filter_update(&filter, 1000.0f, &first) == APOLLO_STATUS_OK, "highpass first update");
	for (int i = 0; i < 40; i++) {
		expect_true(dsp_filter_update(&filter, 1000.0f, &output) == APOLLO_STATUS_OK, "highpass update");
	}

	expect_true(first > output, "highpass step decays");
	expect_true(output > 0.0f, "highpass remains positive during decay");
}

static void test_moving_average(void) {
	dsp_filter_t filter;
	dsp_filter_config_t config;
	float output = 0.0f;

	dsp_filter_default_config(&config);
	config.mode = DSP_FILTER_MODE_MOVING_AVERAGE;
	config.window_size = 4U;

	expect_true(dsp_filter_init(&filter, &config) == APOLLO_STATUS_OK, "moving average init");
	(void)dsp_filter_update(&filter, 1.0f, &output);
	(void)dsp_filter_update(&filter, 2.0f, &output);
	(void)dsp_filter_update(&filter, 3.0f, &output);
	(void)dsp_filter_update(&filter, 4.0f, &output);
	expect_near(output, 2.5f, 0.001f, "moving average full window");
	(void)dsp_filter_update(&filter, 5.0f, &output);
	expect_near(output, 3.5f, 0.001f, "moving average circular window");
}

static void test_median(void) {
	dsp_filter_t filter;
	dsp_filter_config_t config;
	float output = 0.0f;

	dsp_filter_default_config(&config);
	config.mode = DSP_FILTER_MODE_MEDIAN;
	config.window_size = 5U;

	expect_true(dsp_filter_init(&filter, &config) == APOLLO_STATUS_OK, "median init");
	(void)dsp_filter_update(&filter, 100.0f, &output);
	(void)dsp_filter_update(&filter, 1.0f, &output);
	(void)dsp_filter_update(&filter, 3.0f, &output);
	(void)dsp_filter_update(&filter, 2.0f, &output);
	(void)dsp_filter_update(&filter, 4.0f, &output);
	expect_near(output, 3.0f, 0.001f, "median rejects outlier");
}

static void test_invalid_filter_params(void) {
	dsp_filter_t filter;
	dsp_filter_config_t config;

	dsp_filter_default_config(&config);
	config.mode = DSP_FILTER_MODE_EMA;
	config.alpha = 1.5f;
	expect_true(dsp_filter_init(&filter, &config) == APOLLO_STATUS_INVALID_ARG, "invalid ema alpha");

	dsp_filter_default_config(&config);
	config.mode = DSP_FILTER_MODE_MOVING_AVERAGE;
	config.window_size = DSP_FILTER_MOVING_AVERAGE_MAX_WINDOW + 1U;
	expect_true(dsp_filter_init(&filter, &config) == APOLLO_STATUS_INVALID_ARG, "invalid moving average window");
}

static void test_cli_parser(void) {
	apollo_cli_command_t command;

	expect_true(apollo_cli_parse("filter lowpass 250", &command) == APOLLO_STATUS_OK, "parse lowpass");
	expect_true(command.type == APOLLO_CLI_COMMAND_FILTER, "lowpass command type");
	expect_true(command.filter_config.mode == DSP_FILTER_MODE_LOWPASS, "lowpass mode");
	expect_near(command.filter_config.cutoff_hz, 250.0f, 0.001f, "lowpass cutoff");

	expect_true(apollo_cli_parse("input 1", &command) == APOLLO_STATUS_OK, "parse input");
	expect_true(command.type == APOLLO_CLI_COMMAND_INPUT && command.channel == 1U, "input channel");

	expect_true(apollo_cli_parse("demo sine", &command) == APOLLO_STATUS_OK, "parse demo");
	expect_true(command.demo_mode == APOLLO_DEMO_SINE, "demo sine mode");

	expect_true(apollo_cli_parse("input 3", &command) == APOLLO_STATUS_INVALID_ARG, "reject invalid input");
}

static void test_signal_chain_saturation(void) {
	apollo_signal_chain_t chain;
	apollo_signal_sample_t sample;
	dsp_filter_config_t config;

	apollo_signal_chain_init(&chain);
	dsp_filter_default_config(&config);
	config.mode = DSP_FILTER_MODE_BYPASS;
	expect_true(apollo_signal_chain_set_filter(&chain, &config) == APOLLO_STATUS_OK, "signal bypass filter");
	chain.dac_gain = 2.0f;

	expect_true(apollo_signal_chain_process(&chain, 10U, 1U, 3000U, &sample) == APOLLO_STATUS_OK,
				"signal process");
	expect_true(sample.dac_code == APOLLO_DAC_MAX_CODE, "dac saturation high code");
	expect_true((sample.flags & APOLLO_SAMPLE_FLAG_DAC_CLIPPED_HIGH) != 0U, "dac saturation high flag");
}

static void test_diagnostics_and_telemetry(void) {
	apollo_signal_chain_t chain;
	apollo_signal_sample_t sample;
	apollo_diagnostics_t diagnostics;
	char buffer[160];
	int length;

	apollo_signal_chain_init(&chain);
	apollo_diagnostics_reset(&diagnostics);
	(void)apollo_signal_chain_process(&chain, 20U, 2U, 1234U, &sample);
	apollo_diagnostics_update_sample(&diagnostics, &sample);

	expect_true(apollo_diagnostics_mean_adc(&diagnostics) == 1234U, "diagnostics mean");

	length = apollo_telemetry_format_sample(buffer, sizeof(buffer), &sample);
	expect_true(length > 0, "telemetry sample length");
	expect_true(strncmp(buffer, "T,20,2,0,1234,", 14U) == 0, "telemetry sample prefix");

	length = apollo_telemetry_format_status(buffer, sizeof(buffer), &chain, &diagnostics, 1U, 1U);
	expect_true(length > 0, "telemetry status length");
	expect_true(strstr(buffer, "samples=1") != NULL, "telemetry status sample count");
}

int main(void) {
	test_lowpass_step();
	test_highpass_step_decay();
	test_moving_average();
	test_median();
	test_invalid_filter_params();
	test_cli_parser();
	test_signal_chain_saturation();
	test_diagnostics_and_telemetry();

	if (failures != 0) {
		printf("%d host test(s) failed\n", failures);
		return 1;
	}

	printf("host tests passed\n");
	return 0;
}
