#include "app/apollo_app.h"

#include <stdio.h>
#include "app/apollo_config.h"
#include "app/apollo_telemetry.h"
#include "drivers/sd_card.h"
#include "platform/apollo_usb_cdc.h"

static void app_record_usb_result(apollo_app_t *app, apollo_status_t status) {
	if (status == APOLLO_STATUS_BUSY) {
		apollo_diagnostics_record_usb_busy(&app->diagnostics);
	}
}

static void app_send(apollo_app_t *app, const char *text) {
	app_record_usb_result(app, apollo_usb_cdc_write(text));
}

static void app_send_formatted(apollo_app_t *app, char *buffer, int length) {
	if (length <= 0) {
		return;
	}

	if (length >= APOLLO_TELEMETRY_LINE_LENGTH) {
		length = APOLLO_TELEMETRY_LINE_LENGTH - 1;
		buffer[length] = '\0';
	}

	app_record_usb_result(app, apollo_usb_cdc_write(buffer));
}

static void app_send_help(apollo_app_t *app) {
	app_send(app, "commands: help, status, telemetry on|off\r\n");
	app_send(app, "filter bypass|lowpass <hz>|highpass <hz>|ema <alpha>|average <n>|median <n>\r\n");
	app_send(app, "input 0|1, demo off|sine|step|impulse\r\n");
	app_send(app, "calibrate clear|adc <gain> <offset>|dac <gain> <offset>\r\n");
}

static void app_send_status(apollo_app_t *app) {
	char buffer[APOLLO_TELEMETRY_LINE_LENGTH];
	int length = apollo_telemetry_format_status(buffer,
												sizeof(buffer),
												&app->signal_chain,
												&app->diagnostics,
												app->telemetry_enabled,
												apollo_storage_is_available(&app->storage),
												apollo_usb_cdc_rx_overflow_count());
	app_send_formatted(app, buffer, length);
}

static void app_send_command_status(apollo_app_t *app, const char *command, apollo_status_t status) {
	char buffer[APOLLO_TELEMETRY_LINE_LENGTH];
	int length = snprintf(buffer,
						  sizeof(buffer),
						  "%s %s\r\n",
						  command,
						  apollo_status_name(status));
	app_send_formatted(app, buffer, length);
}

static void app_execute_command(apollo_app_t *app, const apollo_cli_command_t *command) {
	apollo_status_t status = APOLLO_STATUS_OK;

	switch (command->type) {
	case APOLLO_CLI_COMMAND_NONE:
		break;

	case APOLLO_CLI_COMMAND_HELP:
		app_send_help(app);
		break;

	case APOLLO_CLI_COMMAND_STATUS:
		app_send_status(app);
		break;

	case APOLLO_CLI_COMMAND_FILTER:
		status = apollo_signal_chain_set_filter(&app->signal_chain, &command->filter_config);
		app_send_command_status(app, "filter", status);
		break;

	case APOLLO_CLI_COMMAND_INPUT:
		status = apollo_adc_set_channel(&app->adc, command->channel);
		if (status == APOLLO_STATUS_OK) {
			status = apollo_signal_chain_set_channel(&app->signal_chain, command->channel);
		}
		app_send_command_status(app, "input", status);
		break;

	case APOLLO_CLI_COMMAND_DEMO:
		apollo_signal_chain_set_demo(&app->signal_chain, command->demo_mode);
		app_send_command_status(app, "demo", APOLLO_STATUS_OK);
		break;

	case APOLLO_CLI_COMMAND_TELEMETRY:
		app->telemetry_enabled = command->telemetry_enabled;
		app_send_command_status(app, "telemetry", APOLLO_STATUS_OK);
		break;

	case APOLLO_CLI_COMMAND_CALIBRATE_CLEAR:
		apollo_signal_chain_clear_calibration(&app->signal_chain);
		app_send_command_status(app, "calibrate", APOLLO_STATUS_OK);
		break;

	case APOLLO_CLI_COMMAND_CALIBRATE_ADC:
		status = apollo_signal_chain_set_adc_calibration(&app->signal_chain,
														 command->cal_gain,
														 command->cal_offset);
		app_send_command_status(app, "calibrate adc", status);
		break;

	case APOLLO_CLI_COMMAND_CALIBRATE_DAC:
		status = apollo_signal_chain_set_dac_calibration(&app->signal_chain,
														 command->cal_gain,
														 command->cal_offset);
		app_send_command_status(app, "calibrate dac", status);
		break;

	default:
		app_send_command_status(app, "command", APOLLO_STATUS_INVALID_ARG);
		break;
	}
}

static void app_poll_commands(apollo_app_t *app) {
	char line[APOLLO_CLI_LINE_LENGTH];
	apollo_cli_command_t command;
	apollo_status_t status;

	while (apollo_usb_cdc_read_line(line, sizeof(line)) == APOLLO_STATUS_OK) {
		status = apollo_cli_parse(line, &command);
		if (status == APOLLO_STATUS_OK) {
			app_execute_command(app, &command);
		} else {
			app_send_command_status(app, "command", status);
		}
	}
}

static void app_process_sample(apollo_app_t *app, uint32_t now_ms) {
	uint16_t raw_adc = 0U;
	apollo_signal_sample_t sample;
	apollo_status_t status;

	if (app->signal_chain.demo_mode == APOLLO_DEMO_OFF) {
		status = apollo_adc_read(&app->adc, &raw_adc);
		if (status != APOLLO_STATUS_OK) {
			apollo_diagnostics_record_adc_error(&app->diagnostics);
			raw_adc = 0U;
		}
	} else {
		raw_adc = apollo_signal_chain_demo_sample(&app->signal_chain);
	}

	status = apollo_signal_chain_process(&app->signal_chain,
										 now_ms,
										 app->sequence++,
										 raw_adc,
										 &sample);
	if (status != APOLLO_STATUS_OK) {
		sample.flags |= APOLLO_SAMPLE_FLAG_FILTER_ERROR;
	}

	status = dac_driver_write(&app->dac, sample.dac_code);
	if (status != APOLLO_STATUS_OK) {
		sample.flags |= APOLLO_SAMPLE_FLAG_DAC_ERROR;
		apollo_diagnostics_record_dac_error(&app->diagnostics);
	}

	status = apollo_storage_log_sample(&app->storage, &sample);
	if (status != APOLLO_STATUS_OK && status != APOLLO_STATUS_UNSUPPORTED) {
		sample.flags |= APOLLO_SAMPLE_FLAG_SRAM_ERROR;
		apollo_diagnostics_record_sram_error(&app->diagnostics);
	}

	apollo_diagnostics_update_sample(&app->diagnostics, &sample);

	if (app->telemetry_enabled != 0U &&
		(uint32_t)(now_ms - app->last_telemetry_tick) >= APOLLO_USB_TELEMETRY_PERIOD_MS) {
		char buffer[APOLLO_TELEMETRY_LINE_LENGTH];
		int length = apollo_telemetry_format_sample(buffer, sizeof(buffer), &sample);
		app_send_formatted(app, buffer, length);
		app->last_telemetry_tick = now_ms;
	}
}

apollo_status_t apollo_app_init(apollo_app_t *app, const apollo_app_handles_t *handles) {
	apollo_status_t adc_status;
	apollo_status_t dac_status;
	apollo_status_t sram_status;
	uint32_t now_ms;

	if (app == NULL || handles == NULL ||
		handles->adc == NULL || handles->dac_i2c == NULL || handles->sram_spi == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	apollo_signal_chain_init(&app->signal_chain);
	apollo_diagnostics_reset(&app->diagnostics);
	app->sequence = 0U;
	app->telemetry_enabled = 1U;

	adc_status = apollo_adc_init(&app->adc, handles->adc, APOLLO_ADC_TIMEOUT_MS);
	dac_status = dac_driver_init(&app->dac,
								 handles->dac_i2c,
								 DAC_DRIVER_DEFAULT_ADDRESS_7BIT,
								 APOLLO_I2C_TIMEOUT_MS);

	sram_status = sram_23k256_init(&app->sram,
								   handles->sram_spi,
								   SRAM_23K256_CS_GPIO_PORT,
								   SRAM_23K256_CS_PIN,
								   APOLLO_SPI_TIMEOUT_MS);
	if (sram_status == APOLLO_STATUS_OK) {
		sram_status = sram_23k256_test(&app->sram);
	}
	apollo_storage_init(&app->storage, &app->sram, (sram_status == APOLLO_STATUS_OK) ? 1U : 0U);

	(void)sd_card_init(handles->sram_spi);

	now_ms = HAL_GetTick();
	app->last_sample_tick = now_ms;
	app->last_telemetry_tick = now_ms;
	app->initialized = 1U;

	app_send(app, "APOLLO DSP ready\r\n");
	if (adc_status != APOLLO_STATUS_OK) {
		app_send_command_status(app, "adc", adc_status);
	}
	if (dac_status != APOLLO_STATUS_OK) {
		app_send_command_status(app, "dac", dac_status);
	}
	if (sram_status != APOLLO_STATUS_OK) {
		app_send_command_status(app, "sram", sram_status);
	}
	app_send_help(app);

	return APOLLO_STATUS_OK;
}

void apollo_app_task(apollo_app_t *app) {
	uint32_t now_ms;

	if (app == NULL || app->initialized == 0U) {
		return;
	}

	app_poll_commands(app);

	now_ms = HAL_GetTick();
	if ((uint32_t)(now_ms - app->last_sample_tick) >= APOLLO_DEFAULT_SAMPLE_PERIOD_MS) {
		app->last_sample_tick = now_ms;
		app_process_sample(app, now_ms);
	}
}
