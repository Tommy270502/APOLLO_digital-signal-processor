#ifndef APP_APOLLO_CLI_H_
#define APP_APOLLO_CLI_H_

#include <stdint.h>
#include "apollo_status.h"
#include "app/apollo_signal_chain.h"
#include "dsp/dsp_filters.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	APOLLO_CLI_COMMAND_NONE = 0,
	APOLLO_CLI_COMMAND_HELP,
	APOLLO_CLI_COMMAND_STATUS,
	APOLLO_CLI_COMMAND_FILTER,
	APOLLO_CLI_COMMAND_INPUT,
	APOLLO_CLI_COMMAND_DEMO,
	APOLLO_CLI_COMMAND_TELEMETRY,
	APOLLO_CLI_COMMAND_CALIBRATE_CLEAR
} apollo_cli_command_type_t;

typedef struct {
	apollo_cli_command_type_t type;
	dsp_filter_config_t filter_config;
	uint8_t channel;
	apollo_demo_mode_t demo_mode;
	uint8_t telemetry_enabled;
} apollo_cli_command_t;

apollo_status_t apollo_cli_parse(const char *line, apollo_cli_command_t *command);

#ifdef __cplusplus
}
#endif

#endif /* APP_APOLLO_CLI_H_ */
