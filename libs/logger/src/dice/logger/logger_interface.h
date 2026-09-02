#ifndef TENTRIS_LOGGER_INTERFACE_H
#define TENTRIS_LOGGER_INTERFACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tentris_log_level {
	TENTRIS_LL_ERROR = 0,
	TENTRIS_LL_WARN  = 1,
	TENTRIS_LL_INFO  = 2,
	TENTRIS_LL_DEBUG = 3,
	TENTRIS_LL_TRACE = 4,
} tentris_log_level;

/**
 * Declaration of log function.
 * The final artifact of your compilation needs to provide a definition
 * for this that logs into your preferred logging framework.
 *
 * @example For an example see tests/TestLogger.cpp
 *
 * @param lvl log level
 * @param function the function this log message originated from
 * @param msg message to send
 */
void tentris_log(tentris_log_level lvl, char const *function, char const *msg);

#ifdef __cplusplus
}
#endif

#endif//TENTRIS_LOGGER_INTERFACE_H
