#ifndef APOLLO_STATUS_H_
#define APOLLO_STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	APOLLO_STATUS_OK = 0,
	APOLLO_STATUS_BUSY,
	APOLLO_STATUS_INVALID_ARG,
	APOLLO_STATUS_HAL_ERROR,
	APOLLO_STATUS_TIMEOUT,
	APOLLO_STATUS_UNSUPPORTED
} apollo_status_t;

const char *apollo_status_name(apollo_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* APOLLO_STATUS_H_ */
