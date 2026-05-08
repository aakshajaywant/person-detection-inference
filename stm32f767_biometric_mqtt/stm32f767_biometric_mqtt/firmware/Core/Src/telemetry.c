#include "telemetry.h"
#include <stdio.h>

int telemetry_format_json(char *out, size_t out_len, const char *device_id, uint32_t seq, const max32664_sample_t *sample, uint32_t uptime_ms)
{
    if (out == NULL || out_len == 0 || device_id == NULL || sample == NULL) return -1;

    uint16_t hr_whole = sample->heart_rate_x10 / 10U;
    uint16_t hr_frac = sample->heart_rate_x10 % 10U;
    const char *finger = max32664_finger_detected(sample) ? "true" : "false";

    return snprintf(out, out_len,
        "{\"device_id\":\"%s\",\"seq\":%lu,\"hr_bpm\":%u.%u,\"spo2_pct\":%u,\"confidence\":%u,\"status\":%u,\"finger_detected\":%s,\"uptime_ms\":%lu}\r\n",
        device_id,
        (unsigned long)seq,
        hr_whole,
        hr_frac,
        sample->spo2,
        sample->confidence,
        sample->status,
        finger,
        (unsigned long)uptime_ms);
}

int telemetry_format_error_json(char *out, size_t out_len, const char *device_id, uint32_t seq, const char *error, uint32_t uptime_ms)
{
    if (out == NULL || out_len == 0 || device_id == NULL || error == NULL) return -1;

    return snprintf(out, out_len,
        "{\"device_id\":\"%s\",\"seq\":%lu,\"error\":\"%s\",\"uptime_ms\":%lu}\r\n",
        device_id,
        (unsigned long)seq,
        error,
        (unsigned long)uptime_ms);
}
