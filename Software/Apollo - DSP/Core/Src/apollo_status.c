#include "apollo_status.h"

const char *apollo_status_name(apollo_status_t status) {
	switch (status) {
	case APOLLO_STATUS_OK:
		return "ok";
	case APOLLO_STATUS_BUSY:
		return "busy";
	case APOLLO_STATUS_INVALID_ARG:
		return "invalid_arg";
	case APOLLO_STATUS_HAL_ERROR:
		return "hal_error";
	case APOLLO_STATUS_TIMEOUT:
		return "timeout";
	case APOLLO_STATUS_UNSUPPORTED:
		return "unsupported";
	default:
		return "unknown";
	}
}
