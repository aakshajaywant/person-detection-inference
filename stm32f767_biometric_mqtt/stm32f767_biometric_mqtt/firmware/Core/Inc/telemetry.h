#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "max32664.h"
#include <stddef.h>
#include <stdint.h>

int telemetry_format_json(char *out, size_t out_len, const char *device_id, uint32_t seq, const max32664_sample_t *sample, uint32_t uptime_ms);
int telemetry_format_error_json(char *out, size_t out_len, const char *device_id, uint32_t seq, const char *error, uint32_t uptime_ms);

#endif /* TELEMETRY_H */
